
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DEBUG_LEVEL 2

#include "ADMFileWriter.h"

BBC_AUDIOTOOLBOX_START

ADMFileWriter::ADMFileWriter() : SoundPositionConsumer(),
                                 ADMRIFFFile()
{
}

ADMFileWriter::~ADMFileWriter()
{
  Close();
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
bool ADMFileWriter::Create(const char *filename, uint32_t samplerate, uint_t nchannels, SampleFormat_t format)
{
  bool success = false;

  if (ADMRIFFFile::Create(filename, samplerate, nchannels, format))
  {
    uint_t i;

    // create cursors for position tracking
    for (i = 0; i < nchannels; i++)
    {
      cursors.push_back(new ADMTrackCursor(i));
    }

    success = true;
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Close file - store final positions before closing file
 *
 * @note this may take some time because it copies sample data from a temporary file
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMFileWriter::Close()
{
  uint64_t t = filesamples ? filesamples->GetPositionNS() : 0;
  uint_t i;

  for (i = 0; i < cursors.size(); i++)
  {
    cursors[i]->Seek(t);
    cursors[i]->EndPositionChanges();
    delete cursors[i];
  }

  ADMRIFFFile::Close();
}

/*--------------------------------------------------------------------------------*/
/** Consume audio
 *
 * @param src source buffer
 * @param nsrcchannels number channels in source buffer
 * @param nsrcframes number of sample frames in source buffer
 */
/*--------------------------------------------------------------------------------*/
void ADMFileWriter::Consume(const Sample_t *src, uint_t nsrcchannels, uint_t nsrcframes)
{
  if (filesamples)
  {
    // write to file
    filesamples->WriteSamples(src, consumestartchannel, nsrcchannels, nsrcframes);
  }
}

/*--------------------------------------------------------------------------------*/
/** Consume audio
 *
 * @param src source buffer
 * @param srcformat format of source buffer
 * @param nsrcchannels number channels in source buffer
 * @param nsrcframes number of sample frames in source buffer
 *
 * @note this is just a wrapper for the above but derived classes can either override this or the above
 */
/*--------------------------------------------------------------------------------*/
void ADMFileWriter::Consume(const uint8_t *src, SampleFormat_t srcformat, uint_t nsrcchannels, uint_t nsrcframes)
{
  if (filesamples)
  {
    // write to file
    filesamples->WriteSamples(src, srcformat, consumestartchannel, nsrcchannels, nsrcframes);
  }
}

/*--------------------------------------------------------------------------------*/
/** Overridable update position function 
 *
 * @param channel channel to change the position of
 * @param pos new position
 * @param supplement optional extra information
 *
 * @note this is the function that should be overridden in derived objects
 */
/*--------------------------------------------------------------------------------*/
void ADMFileWriter::UpdatePositionEx(uint_t channel, const Position& pos, const ParameterSet *supplement)
{
  if (channel < cursors.size())
  {
    PositionCursor *cursor = cursors[channel];

    if (filesamples) cursor->Seek(filesamples->GetPositionNS());
    cursor->SetPosition(pos, supplement);
  }
}

BBC_AUDIOTOOLBOX_END
