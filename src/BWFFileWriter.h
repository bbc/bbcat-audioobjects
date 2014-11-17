#ifndef __BWF_FILE_WRITER__
#define __BWF_FILE_WRITER__

#include <bbcat-render/SoundConsumer.h>

#include "RIFFFile.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Self-registering thin class for writing BWF files
 */
/*--------------------------------------------------------------------------------*/
class BWFFileWriter : public SoundConsumer, public RIFFFile
{
  SELF_REGISTER_DEF(BWFFileWriter);

public:
  BWFFileWriter();
  virtual ~BWFFileWriter();

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
  virtual uint_t GetDesiredOutputChannels() const {return GetChannels();}

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
};

BBC_AUDIOTOOLBOX_END

#endif
