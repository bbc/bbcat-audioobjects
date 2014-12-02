
#include <math.h>

#define DEBUG_LEVEL 2
#include <bbcat-render/SelfRegisteringControlReceiver.h>
#include "ADMFileWriter.h"

BBC_AUDIOTOOLBOX_START

SELF_REGISTERING_CONTROL_RECEIVER(ADMFileWriter, TYPE_ADMBWF ".writer");

static const PARAMETERDESC _parameters[] = 
{
  {"filename",   "Filename of ADM BWF file to create"},
  {"admfile",    "File containing description of ADM"},
  {"samplerate", "Sample rate of file"},
  {"channels",   "Number of channels in file"},
  {"format",     "Sample format ('16bit', '24bit', '32bit', 'float' or 'double')"},
};

// MUST be in the same order as the above
enum
{
  Parameter_filename = 0,
  Parameter_admfile,
  Parameter_samplerate,
  Parameter_channels,
  Parameter_format,
};

ADMFileWriter::ADMFileWriter() : SoundPositionConsumer(),
                                 usersamplerate(0),
                                 samplerate(0),
                                 userchannels(0),
                                 inputchannels(0),
                                 format(SampleFormat_24bit)
{
}

ADMFileWriter::ADMFileWriter(const ParameterSet& parameters) : SoundPositionConsumer(parameters),
                                                               usersamplerate(0),
                                                               samplerate(0),
                                                               userchannels(0),
                                                               inputchannels(0),
                                                               format(SampleFormat_24bit)
{
  SetParameters(parameters);
}

ADMFileWriter::~ADMFileWriter()
{
}

/*--------------------------------------------------------------------------------*/
/** Set parameters within object (*only* parameters that can be set more than once)
 */
/*--------------------------------------------------------------------------------*/
void ADMFileWriter::SetParameters(const ParameterSet& parameters)
{
  std::string _format;

  SoundPositionConsumer::SetParameters(parameters);

  parameters.Get(_parameters[Parameter_filename].name,   filename);
  parameters.Get(_parameters[Parameter_admfile].name,    admfile);
  parameters.Get(_parameters[Parameter_samplerate].name, usersamplerate);
  parameters.Get(_parameters[Parameter_channels].name,   userchannels);
  if (parameters.Get(_parameters[Parameter_format].name, _format))
  {
    if      (_format == "16bit")  format = SampleFormat_16bit;
    else if (_format == "24bit")  format = SampleFormat_24bit;
    else if (_format == "32bit")  format = SampleFormat_32bit;
    else if (_format == "float")  format = SampleFormat_Float;
    else if (_format == "double") format = SampleFormat_Double;
  }

  if (filename != "")
  {
    EnhancedFile file;

    // test to see if WAV file can be opened for writing
    if (file.fopen(filename.c_str(), "wb"))
    {
      // test to see if ADM file exists
      if ((admfile != "") && !EnhancedFile::exists(admfile.c_str()))
      {
        ERROR("ADM file '%s' doesn't exist (during initialisation)", admfile.c_str());
        InvalidateObject();
      }
    }
    else
    {
      ERROR("Failed to open file '%s' for writing (during initialisation)", filename.c_str());
      InvalidateObject();
    }
  }
  else
  {
    ERROR("No filename specified for file writer");
    InvalidateObject();
  }
}

/*--------------------------------------------------------------------------------*/
/** Get a list of parameters for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMFileWriter::GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list)
{
  SoundPositionConsumer::GetParameterDescriptions(list);

  AddParametersToList(_parameters, NUMBEROF(_parameters), list);
}

/*--------------------------------------------------------------------------------*/
/** Optional call to set the number of input channels to expect
 */
/*--------------------------------------------------------------------------------*/
void ADMFileWriter::SetInputChannels(uint_t n)
{
  if (!file.IsOpen())
  {
    inputchannels = n;
  }
  else ERROR("Trying to set input channels in ADMFileWriter after file has been created");
}

/*--------------------------------------------------------------------------------*/
/** Optional call to set the sample rate of incoming audio
 */
/*--------------------------------------------------------------------------------*/
void ADMFileWriter::SetInputSampleRate(uint32_t sr)
{
  if (!file.IsOpen())
  {
    samplerate = sr;
  }
  else ERROR("Trying to set sample rate in ADMFileWriter after file has been created");
}

/*--------------------------------------------------------------------------------*/
/** Create RIFF file 
 */
/*--------------------------------------------------------------------------------*/
void ADMFileWriter::OpenFileIfNecessary()
{
  uint32_t _samplerate = usersamplerate ? usersamplerate : samplerate;
  uint_t   _channels   = userchannels   ? userchannels   : inputchannels;

  if (!file.IsOpen() &&
      IsObjectValid() &&
      (filename != "") &&
      _samplerate &&
      _channels)
  {
    if (file.Create(filename.c_str(), _samplerate, _channels, format))
    {
      DEBUG1(("Created RIFF file '%s' (sample rate %uHz, %u channels)", filename.c_str(), _samplerate, _channels));

      // set position handler up to receive positions
      SetChannels(_channels);

      if (admfile != "")
      {
        if (file.CreateADM(admfile.c_str()))
        {
          DEBUG1(("Created ADM data from file '%s'", admfile.c_str()));
        }
        else
        {
          ERROR("Failed to create ADM data from file '%s'", admfile.c_str());
          InvalidateObject();
        }
      }
    }
    else
    {
      ERROR("Failed to create RIFF file '%s' (sample rate %uHz, %u channels)", filename.c_str(), samplerate, inputchannels);
      InvalidateObject();
    }
  }
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
  // set number of input channels (for file creation)
  inputchannels = nsrcchannels;

  OpenFileIfNecessary();

  // write to file
  file.WriteSamples(src, consumestartchannel, nsrcchannels, nsrcframes);
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
  // set number of input channels (for file creation)
  inputchannels = nsrcchannels;

  OpenFileIfNecessary();

  // write to file
  file.WriteSamples(src, srcformat, consumestartchannel, nsrcchannels, nsrcframes);
}

/*--------------------------------------------------------------------------------*/
/** Update the position for a channel
 *
 * @param channel channel to change the position of
 * @param pos new position
 * @param supplement optional extra information
 *
 * @note don't override this function unless you want to disable the transform facility, override UpdatePositionEx() instead
 */
/*--------------------------------------------------------------------------------*/
void ADMFileWriter::UpdatePosition(uint_t channel, const Position& pos, const ParameterSet *supplement)
{
  // create file if it has not been done yet
  OpenFileIfNecessary();

  SoundPositionConsumer::UpdatePosition(channel, pos, supplement);
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
  file.SetPosition(channel, pos, supplement);
}

BBC_AUDIOTOOLBOX_END
