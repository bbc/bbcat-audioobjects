#ifndef __ADM_FILE_WRITER__
#define __ADM_FILE_WRITER__

#include <bbcat-render/SoundConsumer.h>

#include "ADMRIFFFile.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Self-registering thin class for writing ADM RIFF files
 */
/*--------------------------------------------------------------------------------*/
class ADMFileWriter : public SoundPositionConsumer
{
public:
  ADMFileWriter();
  ADMFileWriter(const ParameterSet& parameters);
  virtual ~ADMFileWriter();

  /*--------------------------------------------------------------------------------*/
  /** Set parameters within object (*only* parameters that can be set more than once)
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetParameters(const ParameterSet& parameters);

  /*--------------------------------------------------------------------------------*/
  /** Get a list of parameters for this object
   */
  /*--------------------------------------------------------------------------------*/
  static void GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list);

  /*--------------------------------------------------------------------------------*/
  /** Optional call to set the number of input channels to expect
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetInputChannels(uint_t n);

  /*--------------------------------------------------------------------------------*/
  /** Optional call to set the sample rate of incoming audio
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetInputSampleRate(uint32_t sr);

  /*--------------------------------------------------------------------------------*/
  /** Return desired output channels
   *
   * @return number of desired output channels (0 == no limit / not restricted)
   */
  /*--------------------------------------------------------------------------------*/
  virtual uint_t GetDesiredOutputChannels() const {return file.GetChannels();}

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
  virtual void UpdatePosition(uint_t channel, const Position& pos, const ParameterSet *supplement = NULL);

  /*--------------------------------------------------------------------------------*/
  /** Return whether this object wants to be a pre-render consumer and receive positions
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool IsPreRenderConsumer() const {return (admfile != "");}

  /*--------------------------------------------------------------------------------*/
  /** Consume audio
   *
   * @param src source buffer
   * @param nsrcchannels number channels in source buffer
   * @param nsrcframes number of sample frames in source buffer
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Consume(const Sample_t *src, uint_t nsrcchannels, uint_t nsrcframes);

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
  virtual void Consume(const uint8_t *src, SampleFormat_t srcformat, uint_t nsrcchannels, uint_t nsrcframes);

protected:
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
  virtual void UpdatePositionEx(uint_t channel, const Position& pos, const ParameterSet *supplement);

  /*--------------------------------------------------------------------------------*/
  /** Create RIFF file 
   */
  /*--------------------------------------------------------------------------------*/
  virtual void OpenFileIfNecessary();

protected:
  ADMRIFFFile    file;
  std::string    filename;
  std::string    admfile;
  uint32_t       usersamplerate;
  uint32_t       samplerate;
  uint_t         userchannels;
  uint_t         inputchannels;
  SampleFormat_t format;
};

BBC_AUDIOTOOLBOX_END

#endif
