
#include <math.h>

#define DEBUG_LEVEL 2

#include "BWFFileWriter.h"

BBC_AUDIOTOOLBOX_START

static const struct {
  PARAMETERDESC filename;
  PARAMETERDESC samplerate;
  PARAMETERDESC channels;
  PARAMETERDESC format;
} _parameters = 
{
  {"filename",   "Filename of ADM BWF file to create"},
  {"samplerate", "Sample rate of file"},
  {"channels",   "Number of channels in file"},
  {"format",     "Sample format ('16bit', '24bit', '32bit', 'float' or 'double')"},
};

SELF_REGISTER(BWFFileWriter, TYPE_POST_RENDER_CONSUMER ".bwf.writer");

BWFFileWriter::BWFFileWriter() : SoundConsumer(),
                                 RIFFFile()
{
}

BWFFileWriter::BWFFileWriter(const ParameterSet& parameters) : SoundConsumer(),
                                                               RIFFFile()
{
  std::string    filename;
  std::string    admfile;
  uint_t         samplerate = 48000;
  uint_t         channels   = 2;
  std::string    _format    = "24bit";
  SampleFormat_t format     = SampleFormat_24bit;

  if (parameters.Get(_parameters.filename.name, filename) && (filename != ""))
  {
    parameters.Get(_parameters.samplerate.name, samplerate);
    parameters.Get(_parameters.channels.name,   channels);
    parameters.Get(_parameters.format.name,     _format);

    if      (_format == "16bit")  format = SampleFormat_16bit;
    else if (_format == "24bit")  format = SampleFormat_24bit;
    else if (_format == "32bit")  format = SampleFormat_32bit;
    else if (_format == "float")  format = SampleFormat_Float;
    else if (_format == "double") format = SampleFormat_Double;

    if (Create(filename.c_str(), samplerate, channels, format))
    {
      DEBUG1(("Created RIFF file '%s' (sample rate %uHz, %u channels)", filename.c_str(), samplerate, channels));
    }
    else ERROR("Failed to create RIFF file '%s' (sample rate %uHz, %u channels)", filename.c_str(), samplerate, channels);
  }

  SetParameters(parameters);
}

BWFFileWriter::~BWFFileWriter()
{
}

/*--------------------------------------------------------------------------------*/
/** Get a list of parameters for this object
 */
/*--------------------------------------------------------------------------------*/
void BWFFileWriter::GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list)
{
  const PARAMETERDESC *pparameters = (const PARAMETERDESC *)&_parameters;
  uint_t i, n = sizeof(_parameters) / sizeof(pparameters[0]);

  SoundConsumer::GetParameterDescriptions(list);

  for (i = 0; i < n; i++) list.push_back(pparameters + i);
}

/*--------------------------------------------------------------------------------*/
/** Consume audio
 *
 * @param src source buffer
 * @param nsrcchannels number channels in source buffer
 * @param nsrcframes number of sample frames in source buffer
 */
/*--------------------------------------------------------------------------------*/
void BWFFileWriter::Consume(const Sample_t *src, uint_t nsrcchannels, uint_t nsrcframes)
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
void BWFFileWriter::Consume(const uint8_t *src, SampleFormat_t srcformat, uint_t nsrcchannels, uint_t nsrcframes)
{
  if (filesamples)
  {
    // write to file
    filesamples->WriteSamples(src, srcformat, consumestartchannel, nsrcchannels, nsrcframes);
  }
}

BBC_AUDIOTOOLBOX_END
