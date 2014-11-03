
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DEBUG_LEVEL 2

#include "ADMFileWriter.h"

BBC_AUDIOTOOLBOX_START

SELF_REGISTER(ADMFileWriter, TYPE_RIFFFILE ".writer");

ADMFileWriter::ADMFileWriter() : SoundPositionConsumer(),
                                 ADMRIFFFile()
{
}

ADMFileWriter::ADMFileWriter(const ParameterSet& parameters) : SoundPositionConsumer(),
                                                               ADMRIFFFile()
{
  std::string    filename;
  std::string    admfile;
  uint_t         samplerate = 48000;
  uint_t         channels   = 2;
  std::string    _format    = "24bit";
  SampleFormat_t format     = SampleFormat_24bit;

  if (parameters.Get("filename", filename) && (filename != ""))
  {
    parameters.Get("admfile",    admfile);
    parameters.Get("samplerate", samplerate);
    parameters.Get("channels",   channels);
    parameters.Get("format",     _format);

    if      (_format == "16bit")  format = SampleFormat_16bit;
    else if (_format == "24bit")  format = SampleFormat_24bit;
    else if (_format == "32bit")  format = SampleFormat_32bit;
    else if (_format == "float")  format = SampleFormat_Float;
    else if (_format == "double") format = SampleFormat_Double;

    if (Create(filename.c_str(), samplerate, channels, format))
    {
      DEBUG1(("Created RIFF file '%s' (sample rate %uHz, %u channels)", filename.c_str(), samplerate, channels));

      if (admfile != "")
      {
        if (CreateADM(admfile.c_str()))
        {
          DEBUG1(("Created ADM data from file '%s'", admfile.c_str()));
        }
        else ERROR("Failed to create ADM data from file '%s'", admfile.c_str());
      }
    }
    else ERROR("Failed to create RIFF file '%s' (sample rate %uHz, %u channels)", filename.c_str(), samplerate, channels);
  }
}

ADMFileWriter::~ADMFileWriter()
{
  Close();
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
/** Create ADM from text file
 *
 * @param filename text filename (see below for format)
 *
 * @return true if successful
 *
 * The file MUST be of the following format with each entry on its own line:
 * <ADM programme name>[:<ADM content name>]
 *
 * then for each track:
 * <track>:<trackname>:<objectname>
 *
 * Where <track> is 1..number of tracks available within ADM
 */
/*--------------------------------------------------------------------------------*/
bool ADMFileWriter::CreateADM(const char *filename)
{
  bool success = false;

  if (ADMRIFFFile::CreateADM(filename))
  {
    std::vector<const ADMAudioObject *> objects;
    uint_t i, nchannels = GetChannels();

    // get list of ADMAudioObjects
    adm->GetAudioObjectList(objects);

    // set position handler up to receive positions
    SetChannels(nchannels);

    // create cursors for position tracking
    // add all objects to all cursors
    for (i = 0; i < nchannels; i++)
    {
      ADMTrackCursor *cursor;

      if ((cursor = new ADMTrackCursor(i)) != NULL) {
        cursors.push_back(cursor);
        cursor->Add(objects);
      }
    }

    success = true;
  }

  return success;
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
