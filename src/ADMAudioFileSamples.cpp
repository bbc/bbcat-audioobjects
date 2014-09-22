
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEBUG_LEVEL 2
#include "ADMAudioFileSamples.h"

BBC_AUDIOTOOLBOX_START

ADMAudioFileSamples::ADMAudioFileSamples(const ADMData *iadm, const SoundFileSamples *isamples, const ADMAudioObject *obj) : SoundFileSamplesWithPosition(isamples),
                                                                                                                             adm(iadm)
{
  if (obj)
  {
    Clip_t newclip;

    newclip.start = ADMObject::TimeToSamples(obj->GetChildrenStartTime(), format->GetSampleRate());
    if (obj->GetChildrenEndTime() == obj->GetChildrenStartTime())
    {
      newclip.nsamples = ~(uint64_t)0;
    }
    else
    {
      newclip.nsamples = ADMObject::TimeToSamples(obj->GetChildrenEndTime(),   format->GetSampleRate()) - newclip.start;
    }
    newclip.channel   = obj->GetChildrenStartChannel();
    newclip.nchannels = obj->GetChildrenChannelCount();

    SetClip(newclip);
  }

  adm->CreateCursors(cursors, GetClip().channel, GetClip().nchannels);

  DEBUG2(("Channels %u for %u, samples %lu for %lu (%s to %s)",
          GetClip().channel, GetClip().nchannels,
          (ulong_t)GetClip().start, (ulong_t)GetClip().nsamples,
          ADMObject::GenTime(ADMObject::SamplesToTime(GetClip().start,                      format->GetSampleRate())).c_str(),
          ADMObject::GenTime(ADMObject::SamplesToTime(GetClip().start + GetClip().nsamples, format->GetSampleRate())).c_str()));
}

ADMAudioFileSamples::ADMAudioFileSamples(const ADMAudioFileSamples *isamples) : SoundFileSamplesWithPosition(isamples),
                                                                                adm(isamples->adm)
{
  adm->CreateCursors(cursors, GetClip().channel, GetClip().nchannels);

  DEBUG2(("Channels %u for %u, samples %lu for %lu (%s to %s)",
          GetClip().channel, GetClip().nchannels,
          (ulong_t)GetClip().start, (ulong_t)GetClip().nsamples,
          ADMObject::GenTime(ADMObject::SamplesToTime(GetClip().start,                      format->GetSampleRate())).c_str(),
          ADMObject::GenTime(ADMObject::SamplesToTime(GetClip().start + GetClip().nsamples, format->GetSampleRate())).c_str()));
}
                                                                                                     
ADMAudioFileSamples::~ADMAudioFileSamples()
{
}

void ADMAudioFileSamples::UpdatePosition()
{
  uint_t i;

  SoundFileSamples::UpdatePosition();

  for (i = 0; i < cursors.size(); i++)
  {
    cursors[i]->Seek(GetAbsolutePositionNS());
  }
}

BBC_AUDIOTOOLBOX_END
