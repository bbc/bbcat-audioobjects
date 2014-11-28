#ifndef __ADM_FILE_WRITER__
#define __ADM_FILE_WRITER__

#include <bbcat-render/SoundConsumer.h>

#include "ADMRIFFFile.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Self-registering thin class for writing ADM RIFF files
 */
/*--------------------------------------------------------------------------------*/
class ADMFileWriter : public SoundPositionConsumer, public ADMRIFFFile
{
public:
  ADMFileWriter();
  ADMFileWriter(const ParameterSet& parameters);
  virtual ~ADMFileWriter();

  /*--------------------------------------------------------------------------------*/
  /** Get a list of parameters for this object
   */
  /*--------------------------------------------------------------------------------*/
  static void GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list);

  /*--------------------------------------------------------------------------------*/
  /** Return desired output channels
   *
   * @return number of desired output channels (0 == no limit / not restricted)
   */
  /*--------------------------------------------------------------------------------*/
  virtual uint_t GetDesiredOutputChannels() const {return ADMRIFFFile::GetChannels();}

  /*--------------------------------------------------------------------------------*/
  /** Return whether this object wants to be a pre-render consumer and receive positions
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool IsPreRenderConsumer() const {return (adm != NULL);}

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
};

BBC_AUDIOTOOLBOX_END

#endif
