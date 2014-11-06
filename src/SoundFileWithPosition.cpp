
#define DEBUG_LEVEL 1
#include "SoundFileWithPosition.h"

BBC_AUDIOTOOLBOX_START

SoundFileSamplesWithPosition::SoundFileSamplesWithPosition() : SoundFileSamples()
{
}

SoundFileSamplesWithPosition::SoundFileSamplesWithPosition(const SoundFileSamples *obj) : SoundFileSamples(obj)
{
}

SoundFileSamplesWithPosition::~SoundFileSamplesWithPosition()
{
  uint_t i;

  for (i = 0; i < cursors.size(); i++)
  {
    delete cursors[i];
  }
}   

void SoundFileSamplesWithPosition::UpdatePosition()
{
  uint_t i;

  SoundFileSamples::UpdatePosition();

  for (i = 0; i < cursors.size(); i++)
  {
    if (cursors[i]->Seek(GetAbsolutePositionNS()))
    {
      // position changed!
    }
  }
}

BBC_AUDIOTOOLBOX_END
