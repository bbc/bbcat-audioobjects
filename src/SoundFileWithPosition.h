#ifndef __SOUND_FILE_WITH_POSITION__
#define __SOUND_FILE_WITH_POSITION__

#include <vector>

#include <bbcat-base/misc.h>
#include <bbcat-base/PositionCursor.h>
#include <bbcat-base/3DPosition.h>

#include "SoundFileAttributes.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Sound samples object which understands positions - syncs position updates
 * with the reading of audio samples
 */
/*--------------------------------------------------------------------------------*/
class SoundFileSamplesWithPosition : public SoundFileSamples
{
public:
  SoundFileSamplesWithPosition();
  SoundFileSamplesWithPosition(const SoundFileSamples *obj);
  virtual ~SoundFileSamplesWithPosition();

  virtual const std::vector<PositionCursor *>& GetCursors() const {return cursors;}

protected:
  virtual void UpdatePosition();

protected:
  std::vector<PositionCursor *> cursors;
};

BBC_AUDIOTOOLBOX_END

#endif
