
#define DEBUG_LEVEL 1
#include "SoundObjectFile.h"

BBC_AUDIOTOOLBOX_START

SoundObjectFileSamples::SoundObjectFileSamples() : SoundFileSamples()
{
}

SoundObjectFileSamples::SoundObjectFileSamples(const SoundFileSamples *obj) : SoundFileSamples(obj)
{
}

SoundObjectFileSamples::SoundObjectFileSamples(const SoundObjectFileSamples *obj) : SoundFileSamples(obj)
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

void SoundObjectFileSamples::GetObjectList(AudioObject::LIST& list)
{
  AudioObject *lastobj = NULL;
  uint_t i;

  for (i = 0; i < cursors.size(); i++)
  {
    AudioObject *obj = cursors[i]->GetAudioObject();

    if (obj && (obj != lastobj)) list.push_back(obj);
    
    lastobj = obj;
  }  
}

BBC_AUDIOTOOLBOX_END
