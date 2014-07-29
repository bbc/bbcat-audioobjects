
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_LEVEL 1
#include <aplibs-dsp/ByteSwap.h>

#include "RIFFChunk.h"
#include "RIFFFile.h"

BBC_AUDIOTOOLBOX_START

std::map<uint32_t, RIFFChunk::PROVIDER> RIFFChunk::providermap;

/*--------------------------------------------------------------------------------*/
/** Constructor - can only be called by static member function!
 */
/*--------------------------------------------------------------------------------*/
RIFFChunk::RIFFChunk(uint32_t chunk_id) : id(chunk_id),
                                          length(0),
                                          datapos(0),
                                          data(NULL)
{
  // create ASCII name from ID
  char _name[] = {(char)(id >> 24), (char)(id >> 16), (char)(id >> 8), (char)id};

  name.assign(_name, sizeof(_name));
}

RIFFChunk::~RIFFChunk()
{
  DeleteData();
}

/*--------------------------------------------------------------------------------*/
/** Read chunk data length and decide what to do
 *
 * @return true if chunk successfully read/handled
 */
/*--------------------------------------------------------------------------------*/
bool RIFFChunk::ReadChunk(SoundFile *file)
{
  bool success = false;

  // chunk ID has already been read, next is chunk length
  if (file && (file->fread(&length, sizeof(length), 1) == 1))
  {
    // length is stored little-endian
    ByteSwap(length, SWAP_FOR_LE);

    // save file position
    datapos = file->ftell();

    DEBUG2(("Chunk '%s' is %lu bytes long", GetName(), (ulong_t)length));

    // Process chunk
    switch (GetChunkHandling())
    {
      default:
      case ChunkHandling_SkipOverChunk:
        // skip to end of chunk
        if (file->fseek(datapos + length, SEEK_SET) == 0) success = true;
        else
        {
          ERROR("Failed to seek to end of chunk '%s' (position %lu), error %s", GetName(), datapos + length, strerror(file->ferror()));
        }
                
        break;

      case ChunkHandling_RemainInChunkData:
        // remain at current position
        success = true;
        break;

      case ChunkHandling_ReadChunk:
        DEBUG2(("Reading and processing chunk '%s'", GetName()));

        // read and process chunk
        if (ReadData(file))
        {
          // process data
          success = ProcessChunkData();

          // if data is not needed after processing, delete it
          if (DeleteDataAfterProcessing())
          {
            DeleteData();
          }
        }
        else
        {
          // failed to read data, skip over it for next chunk
          ERROR("Failed to read %lu bytes of chunk '%s', error %s", (ulong_t)length, GetName(), strerror(file->ferror()));

          if (file->fseek(datapos + length, SEEK_SET) != 0)
          {
            ERROR("Failed to seek to end of chunk '%s' (position %lu) (after chunk read failure), error %s", GetName(), datapos + length, strerror(file->ferror()));
          }
        }
        break;
    }
  }
  else ERROR("Failed to read chunk '%s' length, error %s", GetName(), strerror(file->ferror()));

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Read chunk data and byte swap it (derived object provided)
 *
 * @return true if chunk successfully read
 */
/*--------------------------------------------------------------------------------*/
bool RIFFChunk::ReadData(SoundFile *file)
{
  bool success = false;

  if (!data && length)
  {
    // allocate data for chunk data
    if ((data = new uint8_t[length]) != NULL)
    {
      // seek to correct position in file (will probably already be there)
      if (file && (file->fseek(datapos, SEEK_SET) == 0))
      {
        // read chunk data
        if (file->fread(data, length, 1) == 1)
        {
          // swap byte ordering for data
          ByteSwapData();

          success = true;
        }
        else ERROR("Failed to read %lu bytes for chunk '%s' data, error %s", (ulong_t)length, GetName(), strerror(file->ferror()));
      }
      else ERROR("Failed to seek to position %lu to read %lu bytes for chunk '%s' data, error %s", datapos, (ulong_t)length, GetName(), strerror(file->ferror()));
    }
    else ERROR("Failed to allocate %lu bytes for chunk '%s' data", (ulong_t)length, GetName());
  }
  else success = true;

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Delete read data
 */
/*--------------------------------------------------------------------------------*/
void RIFFChunk::DeleteData()
{
  if (data)
  {
    delete[] data;
    data = NULL;
  }
}

/*--------------------------------------------------------------------------------*/
/** Register a chunk handler
 *
 * @param id 32-bit chunk ID (big-endian format)
 * @param fn function to create RIFFChunk derived object
 * @param context optional userdata to be supplied to the above function
 *
 */
/*--------------------------------------------------------------------------------*/
void RIFFChunk::RegisterProvider(uint32_t id, RIFFChunk *(*fn)(uint32_t id, void *context), void *context)
{
  PROVIDER provider =
  {
    .fn      = fn,          // creator function
    .context = context,     // user supplied data for creator
  };

  // save creator against chunk ID
  providermap[id] = provider;
}

/*--------------------------------------------------------------------------------*/
/** Register a chunk handler
 *
 * @param name ASCII chunk name (optionally terminated)
 * @param fn function to create RIFFChunk derived object
 * @param context optional userdata to be supplied to the above function
 *
 */
/*--------------------------------------------------------------------------------*/
void RIFFChunk::RegisterProvider(const char *name, RIFFChunk *(*fn)(uint32_t id, void *context), void *context)
{
  RegisterProvider(IFFID(name), fn, context);
}

/*--------------------------------------------------------------------------------*/
/** Return ASCII name representation of chunk ID
 *
 * @note the return is an allocated string (using CreateString()) and must be freed
 * @note at some point by calling FreeStrings()
 */
/*--------------------------------------------------------------------------------*/
const char *RIFFChunk::GetChunkName(uint32_t id)
{
  char array[] = {(char)(id >> 24), (char)(id >> 16), (char)(id >> 8), (char)id};
  return CreateString(array, sizeof(array));
}

/*--------------------------------------------------------------------------------*/
/** The primary chunk creation function
 *
 * @param file open file positioned at chunk ID point
 *
 * @return RIFFChunk object for the chunk
 *
 * @note at return, the new file position will be at the start of the next chunk
 */
/*--------------------------------------------------------------------------------*/
RIFFChunk *RIFFChunk::Create(SoundFile *file)
{
  RIFFChunk *chunk = NULL;
  uint32_t id;
  bool success = false;

  // read chunk ID
  if (file && (file->fread(&id, sizeof(id), 1) == 1))
  {
    // treat ID as big-endian
    ByteSwap(id, SWAP_FOR_BE);

    // find provider to create RIFFChunk object
    std::map<uint32_t, PROVIDER>::iterator it = providermap.find(id);

    if (it != providermap.end())
    {
      // a provider is available
      const PROVIDER& provider = it->second;

      if ((chunk = (*provider.fn)(id, provider.context)) != NULL)
      {
        DEBUG4(("Found provider for chunk '%s'", GetChunkName(id)));
                
        // let object handle the rest of the chunk
        if (chunk->ReadChunk(file))
        {
          DEBUG4(("Read chunk '%s' successfully", GetChunkName(id)));
                    
          success = true;
        }
      }
    }
    else
    {
      // if no provider is available, use the base-class to provide basic functionality
      DEBUG2(("No handler found for chunk '%s', creating empty one", GetChunkName(id)));

      if ((chunk = new RIFFChunk(id)) != NULL)
      {
        success = chunk->ReadChunk(file);
      }
    }
  }

  // if there was a failure, delete the object
  if (!success && chunk)
  {
    delete chunk;
    chunk = NULL;
  }

  return chunk;
}

bool RIFFChunk::SwapBigEndian()
{
  static const bool swap = !MACHINE_IS_BIG_ENDIAN;
  return swap;        // true if machine is little-endian and therefore big-endian data should be swapped
}

bool RIFFChunk::SwapLittleEndian()
{
  static const bool swap = MACHINE_IS_BIG_ENDIAN;
  return swap;        // true if machine is bit-endian and therefore little-endian data should be swapped
}

BBC_AUDIOTOOLBOX_END
