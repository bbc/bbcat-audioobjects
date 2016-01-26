
#define BBCDEBUG_LEVEL 1
#include "SoundObjectFile.h"

BBC_AUDIOTOOLBOX_START

SoundObjectFileSamples::SoundObjectFileSamples() : SoundFileSamples(),
                                                   adm(NULL)
{
}

SoundObjectFileSamples::SoundObjectFileSamples(const SoundFileSamples *obj) : SoundFileSamples(obj),
                                                                              adm(NULL)
{
}

SoundObjectFileSamples::SoundObjectFileSamples(const SoundObjectFileSamples *obj) : SoundFileSamples(obj),
                                                                                    adm(obj->adm)
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

/*--------------------------------------------------------------------------------*/
/** Get list of audio objects active at *current* time
 */
/*--------------------------------------------------------------------------------*/
void SoundObjectFileSamples::GetObjectList(AudioObject::LIST& list)
{
  AudioObject *lastobj = NULL;
  uint_t i;

  SeekAllCursors();
  
  for (i = 0; i < cursors.size(); i++)
  {
    AudioObject *obj = cursors[i]->GetAudioObject();

    if (obj && (obj != lastobj)) list.push_back(obj);
    
    lastobj = obj;
  }  
}

/*--------------------------------------------------------------------------------*/
/** Seek to current time on all cursors
 */
/*--------------------------------------------------------------------------------*/
void SoundObjectFileSamples::SeekAllCursors()
{
  uint64_t currentns = GetAbsolutePositionNS();
  uint_t i;
  
  for (i = 0; i < cursors.size(); i++)
  {
    cursors[i]->Seek(currentns);
  }
}

BBC_AUDIOTOOLBOX_END
