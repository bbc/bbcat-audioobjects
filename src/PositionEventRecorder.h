#ifndef __POSITION_EVENT_RECORDER__
#define __POSITION_EVENT_RECORDER__

#include <bbcat-base/EnhancedFile.h>
#include <bbcat-render/SoundConsumer.h>

BBC_AUDIOTOOLBOX_START

class PositionEventRecorder : public SoundPositionConsumer
{
public:
  PositionEventRecorder();
  PositionEventRecorder(const ParameterSet& parameters);
  virtual ~PositionEventRecorder();

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
  virtual void UpdatePositionEx(uint_t channel, const Position& pos, const ParameterSet *supplement = NULL);

protected:
  std::string  filename;
  EnhancedFile file;
};

BBC_AUDIOTOOLBOX_END

#endif
