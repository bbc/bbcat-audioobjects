
#include <math.h>

#define DEBUG_LEVEL 1

#include "RIFFFile.h"
#include "RIFFChunk_Definitions.h"

BBC_AUDIOTOOLBOX_START

RIFFFile::RIFFFile() : file(NULL),
                       filetype(FileType_Unknown),
                       fileformat(NULL),
                       filesamples(NULL),
                       writing(false)
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
    if (((file = new EnhancedFile) != NULL) && file->fopen(filename, "rb"))
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
    if (samplerate && nchannels &&
        ((file = new EnhancedFile) != NULL) && file->fopen(filename, "wb+"))
    {
      const uint32_t ids[] = {RIFF_ID, WAVE_ID, fmt_ID, data_ID};
      uint_t i;

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
          success  = true;
        }
        else ERROR("No file format and/or file samples chunks created");
      }
    }

    if (!success) Close();
  }

  return success;
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
  uint_t i;

  if (file)
  {
    if (writing && !abortwrite)
    {
      DEBUG1(("Closing file '%s'...", file->getfilename().c_str()));

      // tell each chunk to create it's data
      for (i = 0; i < chunklist.size(); i++)
      {
        chunklist[i]->CreateWriteData();
      }      

      // now total up all the bytes for each chunk
      uint64_t totalbytes = 0;
      for (i = 0; i < chunklist.size(); i++)
      {
        // ignore RIFF ID since the RIFF size refers to the rest of the content
        if (chunklist[i]->GetID() != RIFF_ID)
        {
          uint64_t bytes = chunklist[i]->GetLengthOnFile();

          DEBUG3(("Chunk '%s' has length %lu bytes", chunklist[i]->GetName(), (ulong_t)bytes));
        
          totalbytes += bytes;
        }
      }

      // test whether RIFF64 file is needed
      if (totalbytes >= RIFFChunk::RIFF_MaxSize)
      {
        const RIFFChunk *waveChunk = GetChunk(WAVE_ID);
        const RIFFChunk *dataChunk = GetChunk(data_ID);
        RIFFds64Chunk   *ds64Chunk;

        DEBUG1(("Switching file to RF64 type"));

        // tell each chunk (that's interested) that the file is going to be a RIFF64
        for (i = 0; i < chunklist.size(); i++)
        {
          chunklist[i]->EnableRIFF64();
        }      

        if (waveChunk && dataChunk)
        {
          // create a ds64 chunk and insert it after the WAVE chunk
          if ((ds64Chunk = new RIFFds64Chunk(ds64_ID)) != NULL)
          {
            ds64Chunk->CreateWriteData();

            // TODO: check other chunks for any over 4GB

            // set sizes expicitly in ds64 chunk
            ds64Chunk->SetRIFFSize(totalbytes);
            ds64Chunk->SetdataSize(dataChunk->GetLength());
            ds64Chunk->SetSampleCount(dataChunk->GetLength() / fileformat->GetBytesPerFrame());

            // add length of ds64 chunk to total RIFF size
            totalbytes += ds64Chunk->GetLengthOnFile();

            // find WAVE chunk and add ds64 chunk AFTER it
            std::vector<RIFFChunk *>::iterator it;
            for (it = chunklist.begin(); it != chunklist.end(); ++it)
            {
              if ((*it)->GetID() == WAVE_ID)
              {
                // move to next slot for insert
                ++it;
                // and break out of loop
                break;
              }
            }

            // insert chunk directly after WAVE chunk
            chunklist.insert(it, ds64Chunk);
            chunkmap[ds64_ID] = ds64Chunk;
          }
          else ERROR("Failed to create ds64 chunk");
        }
        else ERROR("Not all RIFF chunks are present, cannot create ds64 chunk");
      }

      DEBUG3(("Total size %lu bytes", (ulong_t)totalbytes));

      // set total length of RIFF chunk
      chunkmap[RIFF_ID]->CreateChunkData(NULL, totalbytes);

      // actually write the chunks' data
      for (i = 0; i < chunklist.size(); i++)
      {
        RIFFChunk *chunk;

        // all but the data chunk can be written (it MUST be last)
        if ((chunk = chunklist[i])->GetID() != data_ID)
        {
          DEBUG2(("Writing chunk '%s' size %lu bytes (actually %lu bytes)", chunklist[i]->GetName(), (ulong_t)chunklist[i]->GetLength(), (ulong_t)chunklist[i]->GetLengthOnFile()));
          chunk->WriteChunk(file);
        }
      }

      // finally, write the data chunk data
      chunkmap[data_ID]->WriteChunk(file);

      DEBUG1(("Closed file '%s'", file->getfilename().c_str()));
    }

    delete file;
    file = NULL;
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
        (id != RIFF_ID) ||
        (id != WAVE_ID) ||
        (id != fmt_ID)  ||
        (id != data_ID))
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
        ERROR("Failed to create chunk of type '%s'", RIFFChunk::GetChunkName(id));
      }
    }
    else
    {
      ERROR("Cannot create two copies of chunk type '%s'", RIFFChunk::GetChunkName(id));
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
