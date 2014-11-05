
#define DEBUG_LEVEL 1
#include "ADMAudioFileSamples.h"

BBC_AUDIOTOOLBOX_START

ADMAudioFileSamples::ADMAudioFileSamples(const SoundFileSamples *isamples, const ADMAudioObject *obj) : SoundFileSamplesWithPosition(isamples)
{
  uint_t i, n = GetChannels();

  // save initial limits of channels and time
  initialclip = GetClip();

  for (i = 0; i < n; i++)
  {
    cursors.push_back(new ADMTrackCursor(GetStartChannel() + i));
  }

  if (obj) Add(obj);
}

ADMAudioFileSamples::ADMAudioFileSamples(const ADMAudioFileSamples *isamples) : SoundFileSamplesWithPosition(isamples)
{
  uint_t i;

  // save initial limits of channels and time
  initialclip = GetClip();

  for (i = 0; i < isamples->cursors.size(); i++)
  {
    const ADMTrackCursor *cursor;

    if ((cursor = dynamic_cast<const ADMTrackCursor *>(isamples->cursors[i])) != NULL)
    {
      cursors.push_back(new ADMTrackCursor(*cursor));
    }
  }
}

ADMAudioFileSamples::~ADMAudioFileSamples()
{
}

bool ADMAudioFileSamples::Add(const ADMAudioObject *obj)
{
  bool   added = false;
  uint_t i;

  for (i = 0; i < cursors.size(); i++)
  {
    ADMTrackCursor *cursor;

    if ((cursor = dynamic_cast<ADMTrackCursor *>(cursors[i])) != NULL)
    {
      if (cursor->Add(obj))
      {
        Clip_t   clip      = GetClip();
        uint64_t startTime = cursor->GetStartTime() / timebase;        // convert cursor start time from ns to samples
        uint64_t endTime   = cursor->GetEndTime()   / timebase;        // convert cursor end   time from ns to samples

        DEBUG2(("Add object from %lu to %lu on track %u...", (ulong_t)startTime, (ulong_t)endTime, cursor->GetChannel() + 1));

        if (objects.size() == 0)
        {
          // first object brings the audio time limits INWARDS to that of the object
          startTime = MAX(startTime, initialclip.start);
          endTime   = MIN(endTime,   initialclip.start + initialclip.nsamples);
        }
        else
        {
          // subsequent objects push the audio time limits OUTWARDS to that of the object (keeping within the original clip)
          startTime = MIN(startTime, MAX(clip.start, initialclip.start));
          endTime   = MAX(endTime,   MIN(clip.start + clip.nsamples, initialclip.start + initialclip.nsamples));
        }

        clip.start    = startTime;
        clip.nsamples = endTime - startTime,

        DEBUG2(("...clip now from %lu to %lu", (ulong_t)startTime, (ulong_t)endTime));

        SetClip(clip);

        objects.push_back(obj);

        added = true;
      }
    }
  }

  return added;
}

bool ADMAudioFileSamples::Add(const ADMAudioObject *objs[], uint_t n)
{
  bool   added = false;
  uint_t i;

  for (i = 0; i < cursors.size(); i++)
  {
    ADMTrackCursor *cursor;

    if ((cursor = dynamic_cast<ADMTrackCursor *>(cursors[i])) != NULL)
    {
      added |= cursor->Add(objs, n);
    }
  }

  return added;
}

bool ADMAudioFileSamples::Add(const std::vector<const ADMAudioObject *>& objs)
{
  bool   added = false;
  uint_t i;

  for (i = 0; i < cursors.size(); i++)
  {
    ADMTrackCursor *cursor;

    if ((cursor = dynamic_cast<ADMTrackCursor *>(cursors[i])) != NULL)
    {
      added |= cursor->Add(objs);
    }
  }

  return added;
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
