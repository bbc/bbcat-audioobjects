
#include <math.h>

#define DEBUG_LEVEL 3

#include <bbcat-base/BackgroundFile.h>

#include "RIFFFile.h"
#include "RIFFChunk_Definitions.h"

BBC_AUDIOTOOLBOX_START

RIFFFile::RIFFFile() : filetype(FileType_Unknown),
                       fileformat(NULL),
                       filesamples(NULL),
                       writing(false),
                       backgroundwriting(false)
{
  if (sizeof(off_t) < sizeof(uint64_t))
  {
    ERROR("System does *NOT* support 64-bit files!");
  }

  if (RIFFChunk::NoProvidersRegistered())
  {
    DEBUG2(("No RIFF chunk providers registered, registering some..."));
    RegisterRIFFChunkProviders();
  }
}

RIFFFile::~RIFFFile()
{
  Close();
}

bool RIFFFile::ReadChunks(uint64_t maxlength)
{
  bool success = false;

  if (IsOpen())
  {
    EnhancedFile        *file = fileref;
    const RIFFds64Chunk *ds64 = NULL;
    RIFFChunk *chunk;
    uint64_t  startpos = file->ftell();

    success = true;

    while (success &&
           ((file->ftell() - startpos) < maxlength) &&
           ((chunk = RIFFChunk::Create(file, ds64)) != NULL))
    {

      chunklist.push_back(chunk);
      chunkmap[chunk->GetID()] = chunk;

      if ((chunk->GetID() == ds64_ID) && ((ds64 = dynamic_cast<const RIFFds64Chunk *>(chunk)) != NULL))
      {
        DEBUG3(("Found ds64 chunk, will be used to set chunk sizes if necessary"));

        if (maxlength == RIFFChunk::RIFF_MaxSize)
        {
          maxlength = ds64->GetRIFFSize();

          DEBUG2(("Updated RIFF size to %lu bytes", (ulong_t)maxlength));
        }
      }

      if ((dynamic_cast<const SoundFormat *>(chunk)) != NULL)
      {
        fileformat = dynamic_cast<SoundFormat *>(chunk);
        if (filesamples) filesamples->SetFormat(fileformat);

        DEBUG3(("Found format chunk (%s)", chunk->GetName()));
      }

      if ((dynamic_cast<const SoundFileSamples *>(chunk)) != NULL)
      {
        filesamples = dynamic_cast<SoundFileSamples *>(chunk);
        if (fileformat) filesamples->SetFormat(fileformat);

        DEBUG3(("Found data chunk (%s)", chunk->GetName()));
      }

      success = ProcessChunk(chunk);
    }

    if (success)
    {
      success = PostReadChunks();
      if (!success) ERROR("Failed post read chunks processing");
    }
  }

  return success;
}

bool RIFFFile::Open(const char *filename)
{
  bool success = false;

  if (!IsOpen())
  {
    EnhancedFile *file;

    if (((file = (fileref = new EnhancedFile)) != NULL) && file->fopen(filename, "rb"))
    {
      RIFFChunk *chunk;

      if ((chunk = RIFFChunk::Create(file)) != NULL)
      {
        chunklist.push_back(chunk);
        chunkmap[chunk->GetID()] = chunk;

        if ((chunk->GetID() == RIFF_ID) || (chunk->GetID() == RF64_ID))
        {
          filetype = FileType_WAV;
          success  = ReadChunks(chunk->GetLength());
        }
      }
    }

    if (!success) Close();
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Enable/disable background file writing
 *
 * @note can be called at any time to enable/disable
 */
/*--------------------------------------------------------------------------------*/
void RIFFFile::EnableBackgroundWriting(bool enable)
{
  BackgroundFile *file;

  backgroundwriting = enable;

  // if we're writing a file, and it is a background capable file, enable or disable background mode
  if (writing && ((file = dynamic_cast<BackgroundFile *>(fileref.Obj())) != NULL))
  {
    file->EnableBackground(backgroundwriting);
  }
}

/*--------------------------------------------------------------------------------*/
/** Create a WAVE/RIFF file
 *
 * @param filename filename of file to create
 * @param samplerate sample rate of audio
 * @param nchannels number of audio channels
 * @param format sample format of audio in file
 *
 * @return true if file created properly
 */
/*--------------------------------------------------------------------------------*/
bool RIFFFile::Create(const char *filename, uint32_t samplerate, uint_t nchannels, SampleFormat_t format)
{
  bool success = false;

  if (!IsOpen())
  {
    // use background file writing mechanism
    BackgroundFile *file;

    if (samplerate && nchannels &&
        ((file = (new BackgroundFile)) != NULL) && file->fopen(filename, "wb+"))
    {
      const uint32_t ids[] = {RIFF_ID, WAVE_ID, ds64_ID, fmt_ID, data_ID};
      uint_t i;

      // NOTE: file starts in foreground writing mode!
      fileref = file;

      writing = true;

      for (i = 0; i < NUMBEROF(ids); i++)
      {
        if (!AddChunk(ids[i])) break;
      }

      if (i == NUMBEROF(ids))
      {
        if (fileformat && filesamples)
        {
          fileformat->SetSampleRate(samplerate);
          fileformat->SetChannels(nchannels);
          fileformat->SetSampleFormat(format);
          fileformat->SetSamplesBigEndian(false);       // WAVE is little-endian

          filesamples->SetFormat(fileformat);

          filetype = FileType_WAV;

          if (CreateExtraChunks())
          {
            RIFFds64Chunk *ds64;
            if ((ds64 = dynamic_cast<RIFFds64Chunk *>(chunkmap[ds64_ID])) != NULL)
            {
              // tell ds64 chunk the maximum number of chunks that might need a table entry
              // this can be calculated from the number of chunks created
              ds64->SetTableCount((uint_t)chunklist.size() - 5);        // none of the above chunks need a table entry (RIFF and data chunks have dedicated entries in the ds64 chunk)
            }

            WriteChunks(file, false);

            // now switch to background writing mode if enabled
            file->EnableBackground(backgroundwriting);

            success  = true;
          }
          else ERROR("Failed to create extra chunks for file writing");
        }
        else ERROR("No file format and/or file samples chunks created");
      }
      else ERROR("Failed to create chunks (failed chunk: %08lx)", (ulong_t)ids[i]);
    }

    if (!success) Close();
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Write chunk data
 */
/*--------------------------------------------------------------------------------*/
void RIFFFile::WriteChunks(EnhancedFile *file, bool closing)
{
  RIFFChunk *chunk;
  uint_t i;

  file->rewind();

  if (!closing)
  {
    // tell each chunk to create it's data
    for (i = 0; i < chunklist.size(); i++)
    {
      chunklist[i]->CreateWriteData();
    }
  }

  // write chunks that need to be written before samples chunk
  for (i = 0; i < chunklist.size(); i++)
  {
    chunk = chunklist[i];

    // write chunk if it can be (ALL chunks are re-written on close anyway)
    if ((chunk->GetID() != data_ID) && chunk->WriteChunkBeforeSamples())
    {
      DEBUG2(("%s: %s chunk '%s' size %lu bytes at %lu (actually %lu bytes)", closing ? "Closing" : "Creating", chunk->WriteThisChunk() ? "Writing" : "SKIPPING", chunk->GetName(), (ulong_t)chunk->GetLength(), (ulong_t)file->ftell(), (ulong_t)chunk->GetLengthOnFile()));
      chunk->WriteChunk(file);
    }
  }

  // now write (proto) data chunk
  if ((chunk = GetChunk(data_ID)) != NULL)
  {
    DEBUG2(("%s: %s chunk '%s' size %lu bytes at %lu (actually %lu bytes)", closing ? "Closing" : "Creating", chunk->WriteThisChunk() ? "Writing" : "SKIPPING", chunk->GetName(), (ulong_t)chunk->GetLength(), (ulong_t)file->ftell(), (ulong_t)chunk->GetLengthOnFile()));
    chunk->WriteChunk(file);
  }

  if (closing)
  {
    // write chunks that need to be written after samples
    for (i = 0; i < chunklist.size(); i++)
    {
      chunk = chunklist[i];

      if ((chunk->GetID() != data_ID) && !chunk->WriteChunkBeforeSamples())
      {
        DEBUG2(("%s: %s chunk '%s' size %lu bytes at %lu (actually %lu bytes)", closing ? "Closing" : "Creating", chunk->WriteThisChunk() ? "Writing" : "SKIPPING", chunk->GetName(), (ulong_t)chunk->GetLength(), (ulong_t)file->ftell(), (ulong_t)chunk->GetLengthOnFile()));
        chunk->WriteChunk(file);
      }
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Close RIFF file, writing chunks if file was opened for writing
 *
 * @param abortwrite true to abort the writing of file
 *
 * @note this may take some time because it copies sample data from a temporary file
 */
/*--------------------------------------------------------------------------------*/
void RIFFFile::Close(bool abortwrite)
{
  EnhancedFile *file = fileref;
  uint_t i;

  if (file)
  {
    if (writing && !abortwrite)
    {
      BackgroundFile *bfile = dynamic_cast<BackgroundFile *>(fileref.Obj());
      RIFFds64Chunk  *ds64  = dynamic_cast<RIFFds64Chunk *>(GetChunk(ds64_ID));
      RIFFChunk      *chunk;
      const uint64_t maxsize = RIFFChunk::RIFF_MaxSize; // max size of chunks and file before switching to RF64

      DEBUG1(("Closing file '%s'...", file->getfilename().c_str()));

      // now total up all the bytes for each chunk
      uint64_t totalbytes = 0;
      for (i = 0; i < chunklist.size(); i++)
      {
        uint64_t bytes;

        chunk = chunklist[i];

        // update data chunk size
        if (chunk->GetID() == data_ID) chunk->CreateWriteData();

        // ignore RIFF ID since the RIFF size refers to the rest of the content
        if (chunk->GetID() != RIFF_ID)
        {
          bytes = chunk->GetLengthOnFile();

          DEBUG3(("Chunk '%s' has length %lu bytes", chunk->GetName(), (ulong_t)bytes));
          totalbytes += bytes;
        }

        // check whether chunk length (NOT length on file) is too big
        // and needs an entry in the ds64 chunk
        // (obviously if *any* chunk exceeds the maximum size, the file will as well)
        if (((bytes = chunk->GetLength()) >= maxsize) ||
            (ds64 && ((chunk->GetID() == RIFF_ID) ||  // RIFF and data chunks should *always* be in the ds64, if it exists
                      (chunk->GetID() == data_ID))))
        {
          DEBUG3(("Chunk '%s' needs to be in ds64 chunk", chunk->GetName()));
          if (ds64)
          {
            if (!ds64->SetChunkSize(chunk->GetID(), bytes)) ERROR("Failed to set chunk size for '%s' in ds64 chunk", chunk->GetName());

            // set the sample count from the size of the data chunk
            if (chunk->GetID() == data_ID)
            {
              ds64->SetSampleCount(bytes / fileformat->GetBytesPerFrame());
            }
          }
          else ERROR("ds64 chunk needed but doesn't exist");
        }
      }

      // test whether RIFF64 file is needed
      if (totalbytes >= maxsize)
      {
        DEBUG1(("Switching file to RF64 type"));

        // tell each chunk (that's interested) that the file is going to be a RIFF64
        for (i = 0; i < chunklist.size(); i++)
        {
          chunklist[i]->EnableRIFF64();
        }
      }

      DEBUG3(("Total size %lu bytes", (ulong_t)totalbytes));

      // set total length of RIFF chunk
      chunkmap[RIFF_ID]->CreateChunkData(NULL, totalbytes);

      // just before writing all chunks again, switch to foreground mode
      if (bfile) bfile->EnableBackground(false);

      // write/re-write all chunks
      WriteChunks(file, true);

      DEBUG1(("Closed file '%s'", file->getfilename().c_str()));
    }

    fileref = NULL;
  }

  filetype    = FileType_Unknown;
  fileformat  = NULL;
  filesamples = NULL;
  writing     = false;

  for (i = 0; i < chunklist.size(); i++)
  {
    delete chunklist[i];
  }

  chunklist.clear();
  chunkmap.clear();
}

/*--------------------------------------------------------------------------------*/
/** Create and add a chunk to a file being written
 *
 * @param id chunk type ID
 * @param name chunk type name
 *
 * @return pointer to chunk object or NULL
 */
/*--------------------------------------------------------------------------------*/
RIFFChunk *RIFFFile::AddChunk(uint32_t id)
{
  RIFFChunk *chunk = NULL;

  if (writing)
  {
    // ensure none of the chunk types specified below are duplicated
    if ((chunkmap.find(id) == chunkmap.end()) ||
        ((id != RIFF_ID) &&
         (id != WAVE_ID) &&
         (id != fmt_ID)  &&
         (id != ds64_ID) &&
         (id != data_ID)))
    {
      if ((chunk = RIFFChunk::Create(id)) != NULL)
      {
        chunklist.push_back(chunk);
        chunkmap[chunk->GetID()] = chunk;

        // if the chunk is a SoundFormat or SoundFileSamples chunk then use it as such

        SoundFormat *fmtchunk;
        if ((fmtchunk = dynamic_cast<SoundFormat *>(chunk)) != NULL)
        {
          fileformat = fmtchunk;
        }

        SoundFileSamples *smpschunk;
        if ((smpschunk = dynamic_cast<SoundFileSamples *>(chunk)) != NULL)
        {
          filesamples = smpschunk;
        }
      }
      else
      {
        ERROR("Failed to create chunk of type '%s'", RIFFChunk::GetChunkName(id).c_str());
      }
    }
    else
    {
      ERROR("Cannot create two copies of chunk type '%s'", RIFFChunk::GetChunkName(id).c_str());
    }
  }
  else
  {
    ERROR("Cannot add chunk to read only file");
  }

  return chunk;
}

RIFFChunk *RIFFFile::AddChunk(const char *name)
{
  return AddChunk(IFFID(name));
}

BBC_AUDIOTOOLBOX_END
