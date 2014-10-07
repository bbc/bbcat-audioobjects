#ifndef __SOUND_FILE_WITH_POSITION__
#define __SOUND_FILE_WITH_POSITION__

#include <vector>

#include <bbcat-base/misc.h>
#include <bbcat-base/PositionCursor.h>
#include <bbcat-base/3DPosition.h>

#include "SoundFileAttributes.h"

BBC_AUDIOTOOLBOX_START

class SoundFileSamplesWithPosition : public SoundFileSamples
{
public:
  SoundFileSamplesWithPosition() : SoundFileSamples() {}
  SoundFileSamplesWithPosition(const SoundFileSamples *obj) : SoundFileSamples(obj) {}
  virtual ~SoundFileSamplesWithPosition()
  {
    uint_t i;
    for (i = 0; i < cursors.size(); i++)
    {
      delete cursors[i];
    }
  }   

  virtual const std::vector<PositionCursor *>& GetCursors() const {return cursors;}

protected:
  std::vector<PositionCursor *> cursors;
};

BBC_AUDIOTOOLBOX_END

#endif
