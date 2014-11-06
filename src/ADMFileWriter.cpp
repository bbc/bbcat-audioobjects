
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
          // set position handler up to receive positions
          SetChannels(channels);

          DEBUG1(("Created ADM data from file '%s'", admfile.c_str()));
        }
        else ERROR("Failed to create ADM data from file '%s'", admfile.c_str());
      }
    }
    else ERROR("Failed to create RIFF file '%s' (sample rate %uHz, %u channels)", filename.c_str(), samplerate, channels);
  }

  SetParameters(parameters);
}

ADMFileWriter::~ADMFileWriter()
{
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
  ADMRIFFFile::SetPosition(channel, pos, supplement);
}

BBC_AUDIOTOOLBOX_END
