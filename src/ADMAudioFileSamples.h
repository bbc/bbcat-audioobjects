#ifndef __ADM_AUDIO_OBJECT_FILE__
#define __ADM_AUDIO_OBJECT_FILE__

#include "ADMData.h"
#include "ADMObjects.h"
#include "SoundFileWithPosition.h"

BBC_AUDIOTOOLBOX_START

class ADMAudioFileSamples : public SoundFileSamplesWithPosition
{
public:
  ADMAudioFileSamples(const ADMData *iadm, const SoundFileSamples *isamples, const ADMAudioObject *obj = NULL);
  ADMAudioFileSamples(const ADMAudioFileSamples *isamples);
  virtual ~ADMAudioFileSamples();

  virtual ADMAudioFileSamples *Duplicate() const {return new ADMAudioFileSamples(this);}

protected:
  virtual void UpdatePosition();

protected:
  const ADMData *adm;
};

BBC_AUDIOTOOLBOX_END

#endif
