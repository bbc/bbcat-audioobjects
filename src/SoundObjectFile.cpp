
#define DEBUG_LEVEL 1
#include "SoundObjectFile.h"

BBC_AUDIOTOOLBOX_START

SoundObjectFileSamples::SoundObjectFileSamples() : SoundFileSamples()
{
}

SoundObjectFileSamples::SoundObjectFileSamples(const SoundFileSamples *obj) : SoundFileSamples(obj)
{
}

SoundObjectFileSamples::~SoundObjectFileSamples()
{
  uint_t i;

  for (i = 0; i < cursors.size(); i++)
  {
    delete cursors[i];
  }
}   

void SoundObjectFileSamples::UpdatePosition()
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
