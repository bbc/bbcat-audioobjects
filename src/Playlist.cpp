
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_LEVEL 1
#include "Playlist.h"

#include "SoundFileAttributes.h"

BBC_AUDIOTOOLBOX_START

Playlist::Playlist() : filestartpos(0),
                       playlistlength(0),
                       fadesamples(100),
                       fadedowncount(0),
                       fadeupcount(0),
                       loop_all(false),
                       positionchange(false)
{
  it = list.begin();
}

Playlist::~Playlist()
{
  uint_t i;

  for (i = 0; i < list.size(); i++)
  {
    delete list[i];
  }
}

/*--------------------------------------------------------------------------------*/
/** Add file to list
 *
 * @note object will be DELETED on destruction of this object!
 */
/*--------------------------------------------------------------------------------*/
void Playlist::AddFile(SoundFileSamples *file)
{
  list.push_back(file);

  playlistlength += file->GetSampleLength();

  // MUST reset here to ensure 'it' is always valid
  Reset();
}

/*--------------------------------------------------------------------------------*/
/** Clear playlist
 */
/*--------------------------------------------------------------------------------*/
void Playlist::Clear()
{
  while (list.size())
  {
    delete list.back();
    list.pop_back();
  }

  playlistlength = 0;
    
  // MUST reset here to ensure 'it' is always valid
  Reset();
}

/*--------------------------------------------------------------------------------*/
/** Reset to start of playback list
 */
/*--------------------------------------------------------------------------------*/
void Playlist::Reset()
{
  filestartpos   = 0;           // reset to start of playlist
  fadedowncount  = 0;           // stop fade down
  fadeupcount    = fadesamples; // start fade up
  positionchange = false;       // cancel position change

  it = list.begin();
  if (it != list.end()) (*it)->SetSamplePosition(0);
}

/*--------------------------------------------------------------------------------*/
/** Move onto next file (or stop if looping is disabled)
 */
/*--------------------------------------------------------------------------------*/
void Playlist::Next()
{
  if (it != list.end())
  {
    // move position on by current file's length
    filestartpos += (*it)->GetSampleLength();

    // advance along list and reset position of file if not at end of list
    if ((++it) != list.end()) (*it)->SetSamplePosition(0);
    // else if looping enabled then reset to the start of the list
    else if (loop_all) Reset();
  }
}

/*--------------------------------------------------------------------------------*/
/** Return current file or NULL if the end of the list has been reached
 */
/*--------------------------------------------------------------------------------*/
SoundFileSamples *Playlist::GetFile()
{
  return (it != list.end()) ? *it : NULL;
}

/*--------------------------------------------------------------------------------*/
/** Return max number of audio channels of playlist
 */
/*--------------------------------------------------------------------------------*/
uint_t Playlist::GetMaxOutputChannels() const
{
  uint_t i, channels = 0;

  for (i = 0; i < list.size(); i++)
  {
    uint_t file_channels = list[i]->GetChannels();

    channels = MAX(channels, file_channels);
  }

  return channels;
}

/*--------------------------------------------------------------------------------*/
/** Return current playback position (in samples)
 */
/*--------------------------------------------------------------------------------*/
uint64_t Playlist::GetPlaybackPosition() const
{
  uint64_t pos = filestartpos;
  if (it != list.end()) pos += (*it)->GetSamplePosition();
  return pos;
}

/*--------------------------------------------------------------------------------*/
/** Set current playback position (in samples)
 *
 * @note setting force to true may cause clicks!
 * @note setting force to false causes a fade down *before* and a fade up *after*
 * changing the position which means this doesn't actually change the position straight away!
 */
/*--------------------------------------------------------------------------------*/
bool Playlist::SetPlaybackPosition(uint64_t pos, bool force)
{
  ThreadLock lock(tlock);
  bool success = false;

  pos = MIN(pos, playlistlength);

  if (!Empty())
  {
    if (force)
    {
      // clear fade down
      fadedowncount = 0;
      // clear any position change request
      positionchange = false;

      // set position immediately and to hell with fading down
      if (SetPlaybackPositionEx(pos))
      {
        // start fade up
        fadeupcount = fadesamples;
        success     = true;
      }
    }
    else
    {
      // fade down first
      DEBUG2(("Set new position request for position %lu samples", (ulong_t)pos));

      // start fade down
      fadedowncount  = fadesamples;

      // set up for position change
      newposition    = pos;
      positionchange = true;
      success        = true;
    }
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Set current playback position (in samples)
 */
/*--------------------------------------------------------------------------------*/
bool Playlist::SetPlaybackPositionEx(uint64_t pos)
{
  bool success = false;

  // move back if necessary
  while ((it != list.begin()) && (pos < filestartpos))
  {
    // move back
    --it;
    // move position back by new file's length
    filestartpos -= (*it)->GetSampleLength();
  }
  // move forward if necessary
  while ((it != list.end()) && (pos >= (filestartpos + (*it)->GetSampleLength())))
  {
    // move position back by existing file's length
    filestartpos += (*it)->GetSampleLength();
    // move forward
    ++it;
  }

  if ((it != list.end()) && (pos >= filestartpos) && (pos < (filestartpos + (*it)->GetSampleLength())))
  {
    (*it)->SetSamplePosition(pos - filestartpos);
    success = true;
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Read samples into buffer
 *
 * @param dst destination sample buffer
 * @param channel offset channel to read from
 * @param channels number of channels to read
 * @param frames maximum number of frames to read
 *
 * @return actual number of frames read
 */
/*--------------------------------------------------------------------------------*/
uint_t Playlist::ReadSamples(Sample_t *dst, uint_t channel, uint_t channels, uint_t frames)
{
  ThreadLock lock(tlock);
  uint_t nframes = 0;

  while (!AtEnd() && frames)
  {
    uint_t nread = 0;

    if (fadedowncount)
    {
      uint_t i, j;

      DEBUG2(("Fading down: %u frames left, coeff %0.3f", fadedowncount, (Sample_t)(fadedowncount - 1) / (Sample_t)fadesamples));

      // fading down, limit to fadedowncount and fade after reading
      nread = GetFile()->ReadSamples(dst, channel, channels, MIN(frames, fadedowncount));
      DEBUG2(("Read %u/%u/%u frames from file (fadedown)", nread, frames, fadedowncount));

      // fade read audio
      for (i = 0; i < nread; i++, fadedowncount--)
      {
        Sample_t mul = (Sample_t)(fadedowncount - 1) / (Sample_t)fadesamples;
        for (j = 0; j < channels; j++) dst[i * channels + j] *= mul;
      }
    }
    else if (positionchange)
    {
      DEBUG2(("Changing position to %lu samples", (ulong_t)newposition));

      // actually set the new position now that audio is faded down
      SetPlaybackPositionEx(newposition);

      // clear change request
      positionchange = false;

      // start fadeup
      fadeupcount = fadesamples;

      // force loop around
      continue;
    }
    else if (fadeupcount)
    {
      uint_t i, j;

      DEBUG2(("Fading up: %u frames left, coeff %0.3f", fadeupcount, (Sample_t)(fadesamples - fadeupcount) / (Sample_t)fadesamples));

      // fading up, limit to fadeupcount and fade after reading
      nread = GetFile()->ReadSamples(dst, channel, channels, MIN(frames, fadeupcount));
      DEBUG2(("Read %u/%u/%u frames from file (fadeup)", nread, frames, fadeupcount));

      // fade read audio
      for (i = 0; i < nread; i++, fadeupcount--)
      {
        Sample_t mul = (Sample_t)(fadesamples - fadeupcount) / (Sample_t)fadesamples;
        for (j = 0; j < channels; j++) dst[i * channels + j] *= mul;
      }
    }
    else if (GetFile())
    {
      DEBUG2(("Reading %u frames from file", frames));
      // no position changes pending, simple read
      nread = GetFile()->ReadSamples(dst, channel, channels, frames);
      DEBUG2(("Read %u/%u frames from file", nread, frames));
    }
    else ERROR("No file when trying to read samples in Playlist!");

    if (!nread) break;

    dst     += nread * channels;
    frames  -= nread;
    nframes += nread;
  }

  return nframes;
}

BBC_AUDIOTOOLBOX_END
