
#ifndef __RIFF_CHUNK__
#define __RIFF_CHUNK__

#include <map>
#include <string>

#include <aplibs-dsp/misc.h>

#include "SoundFileAttributes.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Class for handling RIFF file chunks (used in WAV/AIFF/AIFC files)
 *
 * This object provides basic chunk handling facilities which can be used for
 * unknown chunks whereas the majority of chunks should use derived versions of this
 *
 * Chunk ID's are stored as 32-bit ints in big endian format, therefore 'RIFF' would
 * be stored as 0x52494646
 *
 * The chunk ID governs what object gets created to handle the data, 'providers' are
 * registered (using RIFFChunk::RegisterProvider()) and must provide a function to
 * create a RIFFChunk derived object to handle the chunk
 *
 * Chunk data is not NECESSARILY read and stored within this object!  It is up to
 * the derived objects to decide whether the chunk data should automatically be
 * read
 *
 * The function ChunkNeedsReading() controls what happens to the chunk:
 *   if it returns true, the chunk data is read, byte swapped and processed
 *   if it returns false, the chunk data is skipped over
 * The chunk data can be read subsequently by calling ReadData() with an open file
 * handle to the file
 * 
 */
/*--------------------------------------------------------------------------------*/
class RIFFFile;
class RIFFChunk
{
public:
  virtual ~RIFFChunk();

  /*--------------------------------------------------------------------------------*/
  /** Return chunk ID as 32-bit, big endian, integer
   */
  /*--------------------------------------------------------------------------------*/
  uint32_t       GetID()     const {return id;}

  /*--------------------------------------------------------------------------------*/
  /** Return chunk name as ASCII string (held internally so there's no need to free the return)
   */
  /*--------------------------------------------------------------------------------*/
  const char    *GetName()   const {return name.c_str();}

  /*--------------------------------------------------------------------------------*/
  /** Return chunk data length
   */
  /*--------------------------------------------------------------------------------*/
  uint32_t       GetLength() const {return length;}

  /*--------------------------------------------------------------------------------*/
  /** Return chunk data (or NULL if data has not yet been read)
   */
  /*--------------------------------------------------------------------------------*/
  const uint8_t *GetData()   const {return data;}

  /*--------------------------------------------------------------------------------*/
  /** Delete read data
   */
  /*--------------------------------------------------------------------------------*/
  virtual void DeleteData();

  /*--------------------------------------------------------------------------------*/
  /** Register a chunk handler
   *
   * @param id 32-bit chunk ID (big-endian format)
   * @param fn function to create RIFFChunk derived object
   * @param context optional userdata to be supplied to the above function
   *
   */
  /*--------------------------------------------------------------------------------*/
  static void RegisterProvider(uint32_t id, RIFFChunk *(*fn)(uint32_t id, void *context), void *context = NULL);

  /*--------------------------------------------------------------------------------*/
  /** Register a chunk handler
   *
   * @param name ASCII chunk name (optionally terminated)
   * @param fn function to create RIFFChunk derived object
   * @param context optional userdata to be supplied to the above function
   *
   */
  /*--------------------------------------------------------------------------------*/
  static void RegisterProvider(const char *name, RIFFChunk *(*fn)(uint32_t id, void *context), void *context = NULL);

  /*--------------------------------------------------------------------------------*/
  /** Return ASCII name representation of chunk ID
   *
   * @note the return is an allocated string (using CreateString()) and must be freed
   * @note at some point by calling FreeStrings()
   */
  /*--------------------------------------------------------------------------------*/
  static const char *GetChunkName(uint32_t id);

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
  static RIFFChunk *Create(SoundFile *file);

protected:
  /*--------------------------------------------------------------------------------*/
  /** Constructor - can only be called by static member function!
   */
  /*--------------------------------------------------------------------------------*/
  RIFFChunk(uint32_t chunk_id);

private:
  RIFFChunk(const RIFFChunk& obj) {(void)obj;}    // do not allow copies

protected:
  /*--------------------------------------------------------------------------------*/
  /** Read chunk data length and decide what to do
   *
   * @return true if chunk successfully read/handled
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool ReadChunk(SoundFile *file);
  /*--------------------------------------------------------------------------------*/
  /** Read chunk data and byte swap it (derived object provided)
   *
   * @return true if chunk successfully read
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool ReadData(SoundFile *file);

  /*--------------------------------------------------------------------------------*/
  /** placeholder for byte swapping function provided by derived objects
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ByteSwapData() {}

  typedef enum
  {
    ChunkHandling_SkipOverChunk,
    ChunkHandling_RemainInChunkData,
    ChunkHandling_ReadChunk,
  } ChunkHandling_t;

  /*--------------------------------------------------------------------------------*/
  /** Returns what to do with the chunk:
   *  SkipOverChunk: Skip over data
   *  RemainInChunkData: Remain at start of data (used for nested chunks)
   *  ReadChunk: Read and process chunk data
   */
  /*--------------------------------------------------------------------------------*/
  virtual ChunkHandling_t GetChunkHandling() const {return ChunkHandling_SkipOverChunk;}

  /*--------------------------------------------------------------------------------*/
  /** placeholder for chunk data processing function provided by derived objects
   *
   * @return false to fail chunk reading
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool ProcessChunkData() {return true;}

  /*--------------------------------------------------------------------------------*/
  /** Return whether to delete data after processing
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool DeleteDataAfterProcessing() const {return false;}

  /*--------------------------------------------------------------------------------*/
  /** Return whether it is necessary to byte swap data
   *
   * SwapBigEndian()    returns true if machine is little endian
   * SwapLittleEndian() returns true if machine is big endian
   *
   * For big-endian data,    use if (SwapBigEndian()) {byte swap data...}
   * For little-endian data, use if (SwapLittleEndian()) {byte swap data...}
   */
  /*--------------------------------------------------------------------------------*/
  static bool SwapBigEndian();
  static bool SwapLittleEndian();

protected:
  /*--------------------------------------------------------------------------------*/
  /** Structure to contain providers to create chunk objects from chunk ID's
   */
  /*--------------------------------------------------------------------------------*/
  typedef struct
  {
    RIFFChunk *(*fn)(uint32_t id, void *context);
    void      *context;
  } PROVIDER;

protected:
  uint32_t    id;             ///< chunk ID
  std::string name;           ///< chunk ID as string
  uint32_t    length;         ///< chunk data length
  ulong_t     datapos;        ///< chunk data file position
  uint8_t     *data;          ///< chunk data (if read)

  static std::map<uint32_t,PROVIDER> providermap;
};

BBC_AUDIOTOOLBOX_END

#endif
