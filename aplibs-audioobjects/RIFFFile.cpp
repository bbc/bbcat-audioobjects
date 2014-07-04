
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DEBUG_LEVEL 1

#include "RIFFFile.h"

using namespace std;

BBC_AUDIOTOOLBOX_START

RIFFFile::RIFFFile() : file(NULL),
                       filetype(FileType_Unknown),
                       fileformat(NULL),
                       filesamples(NULL)
{
}

RIFFFile::~RIFFFile()
{
  Close();
}

bool RIFFFile::ReadChunks(ulong_t maxlength)
{
  bool success = false;

  if (IsOpen()) {
    RIFFChunk *chunk;
    uint32_t  startpos = file->ftell();

    success = true;

    while (success &&
           ((ulong_t)(file->ftell() - startpos) < maxlength) &&
           ((chunk = RIFFChunk::Create(file)) != NULL)) {
      chunklist.push_back(chunk);
      chunkmap[chunk->GetID()] = chunk;

      if ((dynamic_cast<const SoundFormat *>(chunk)) != NULL) {
        fileformat = dynamic_cast<const SoundFormat *>(chunk);
        if (filesamples) filesamples->SetFormat(fileformat);

        DEBUG3(("Found format chunk (%s)", chunk->GetName()));
      }

      if ((dynamic_cast<const SoundFileSamples *>(chunk)) != NULL) {
        filesamples = dynamic_cast<SoundFileSamples *>(chunk);
        if (fileformat) filesamples->SetFormat(fileformat);

        DEBUG3(("Found data chunk (%s)", chunk->GetName()));
      }
            
      success = ProcessChunk(chunk);
    }

    if (success) {
      success = PostReadChunks();
      if (!success) ERROR("Failed post read chunks processing");
    }
  }

  return success;
}

bool RIFFFile::Open(const char *filename)
{
  bool success = false;

  if (!IsOpen()) {
    if (((file = new SoundFile) != NULL) && file->fopen(filename, "rb")) {
      RIFFChunk *chunk;

      if ((chunk = RIFFChunk::Create(file)) != NULL) {
        chunklist.push_back(chunk);
        chunkmap[chunk->GetID()] = chunk;

        if (chunk->GetID() == IFFID("RIFF")) {
          filetype = FileType_WAV;
          success  = ReadChunks(chunk->GetLength());
        }
      }
    }

    if (!success) Close();
  }

  return success;
}

void RIFFFile::Close()
{
  if (file) {
    delete file;
    file = NULL;
  }

  filetype    = FileType_Unknown;
  fileformat  = NULL;
  filesamples = NULL;

  uint_t i;
  for (i = 0; i < chunklist.size(); i++) {
    delete chunklist[i];
  }

  chunklist.clear();
  chunkmap.clear();
}

BBC_AUDIOTOOLBOX_END
