
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DEBUG_LEVEL 2

#include "RIFFFile.h"
#include "RIFFChunk_Definitions.h"

BBC_AUDIOTOOLBOX_START

RIFFFile::RIFFFile() : file(NULL),
                       filetype(FileType_Unknown),
                       fileformat(NULL),
                       filesamples(NULL),
                       writing(false)
{
}

RIFFFile::~RIFFFile()
{
  Close();
}

bool RIFFFile::ReadChunks(ulong_t maxlength)
{
  bool success = false;

  if (IsOpen())
  {
    RIFFChunk *chunk;
    uint32_t  startpos = file->ftell();

    success = true;

    while (success &&
           ((ulong_t)(file->ftell() - startpos) < maxlength) &&
           ((chunk = RIFFChunk::Create(file)) != NULL))
    {
      chunklist.push_back(chunk);
      chunkmap[chunk->GetID()] = chunk;

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
    if (((file = new SoundFile) != NULL) && file->fopen(filename, "rb"))
    {
      RIFFChunk *chunk;

      if ((chunk = RIFFChunk::Create(file)) != NULL)
      {
        chunklist.push_back(chunk);
        chunkmap[chunk->GetID()] = chunk;

        if (chunk->GetID() == IFFID("RIFF"))
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
    if (((file = new SoundFile) != NULL) && file->fopen(filename, "wb+"))
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

void RIFFFile::Close()
{
  uint_t i;

  if (file)
  {
    if (writing)
    {
      // tell each chunk to create it's data
      for (i = 0; i < chunklist.size(); i++)
      {
        chunklist[i]->CreateWriteData();
      }      

      // now total up all the bytes for each chunk
      uint32_t totalbytes = 0;
      for (i = 0; i < chunklist.size(); i++)
      {
        uint32_t bytes = chunklist[i]->GetLength();

        DEBUG2(("Chunk '%s' has length %lu bytes", chunklist[i]->GetName(), (ulong_t)bytes));
        
        totalbytes += bytes;
      }

      DEBUG2(("Total bytes %lu bytes", (ulong_t)totalbytes));

      // set total length of RIFF chunk
      chunkmap[RIFF_ID]->CreateWriteData(NULL, totalbytes);

      // actually write the chunks' data
      for (i = 0; i < chunklist.size(); i++)
      {
        RIFFChunk *chunk;

        // all but the data chunk can be written (it MUST be last)
        if ((chunk = chunklist[i])->GetID() != data_ID)
        {
          chunk->WriteChunk(file);
        }
      }

      // finally, write the data chunk data
      chunkmap[data_ID]->WriteChunk(file);
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
    if ((id != RIFF_ID) || (id != WAVE_ID) || (id != fmt_ID) || (id != data_ID) ||
        (chunkmap.find(id) == chunkmap.end()))
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
