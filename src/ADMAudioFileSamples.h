#ifndef __ADM_AUDIO_OBJECT_FILE__
#define __ADM_AUDIO_OBJECT_FILE__

#include "ADMData.h"
#include "ADMObjects.h"
#include "SoundFileWithPosition.h"

BBC_AUDIOTOOLBOX_START

class ADMAudioFileSamples : public SoundFileSamplesWithPosition
{
public:
  ADMAudioFileSamples(const SoundFileSamples *isamples, const ADMAudioObject *obj = NULL);
  ADMAudioFileSamples(const ADMAudioFileSamples *isamples);
  virtual ~ADMAudioFileSamples();

  virtual bool Add(const ADMAudioObject *obj);
  virtual bool Add(const ADMAudioObject *objs[], uint_t n);
  virtual bool Add(const std::vector<const ADMAudioObject *>& objs);

  virtual ADMAudioFileSamples *Duplicate() const {return new ADMAudioFileSamples(this);}

protected:
  virtual void UpdatePosition();

protected:
  Clip_t                              initialclip;
  std::vector<const ADMAudioObject *> objects;
};

BBC_AUDIOTOOLBOX_END

#endif
