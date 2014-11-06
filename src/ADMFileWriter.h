#ifndef __ADM_FILE_WRITE_RENDERER__
#define __ADM_FILE_WRITE_RENDERER__

#include <bbcat-render/SoundConsumer.h>

#include "ADMRIFFFile.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Self-registering thin class for writing ADM RIFF files
 */
/*--------------------------------------------------------------------------------*/
class ADMFileWriter : public SoundPositionConsumer, public ADMRIFFFile
{
  SELF_REGISTER_CREATOR(ADMFileWriter);

public:
  ADMFileWriter();
  virtual ~ADMFileWriter();

  /*--------------------------------------------------------------------------------*/
  /** Return desired output channels
   *
   * @return number of desired output channels (0 == no limit / not restricted)
   */
  /*--------------------------------------------------------------------------------*/
  virtual uint_t GetDesiredOutputChannels() const {return GetChannels();}

  /*--------------------------------------------------------------------------------*/
  /** Return whether this object actually supports positional information
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool CanHandlePositions() const {return (adm != NULL);}

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
