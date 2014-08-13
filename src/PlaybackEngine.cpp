
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_LEVEL 2
#include "PlaybackEngine.h"
#include "SoundFileWithPosition.h"
#include "FilePositionGenerator.h"
#include "ADMRIFFFile.h"
#include "ADMAudioFileSamples.h"

BBC_AUDIOTOOLBOX_START

PlaybackEngine::PlaybackEngine() : AudioPositionProcessor()
{
  samplesbuffer.resize(4096);
  SetGenerator(new FilePositionGenerator(this, playlist));
}

PlaybackEngine::~PlaybackEngine()
{
}

/*--------------------------------------------------------------------------------*/
/** Add file to playlist
 *
 * @note object will be DELETED on destruction of this object!
 */
/*--------------------------------------------------------------------------------*/
void PlaybackEngine::AddFile(SoundFileSamples *file)
{
  ThreadLock lock(tlock);
  bool empty = playlist.Empty();

  playlist.AddFile(file);

  if (empty)
  {
    // first file added, set up renderer
    SetFileChannelsAndSampleRate();
  }
}

/*--------------------------------------------------------------------------------*/
/** Add audio object to playlist
 *
 * @param file open audio file support audio objects
 * @param name name of audio object or 'all' for entire file
 */
/*--------------------------------------------------------------------------------*/
bool PlaybackEngine::AddObject(const ADMRIFFFile& file, const char *name)
{
  ThreadLock lock(tlock);
  const ADMData *adm;
  bool success = false;

  if ((adm = file.GetADM()) != NULL)
  {
    if (strcasecmp(name, "all") == 0)
    {
      ADMAudioFileSamples *afile;
    
      // play entire file
      if ((afile = new ADMAudioFileSamples(adm, file.GetSamples())) != NULL)
      {
        DEBUG2(("Adding entire audio file (%u channels, %lu samples) to list", afile->GetChannels(), afile->GetLength()));
        AddFile(afile);
        success = true;
      }
      else ERROR("Unable to duplicate file handle for '%s'", name);
    }
    else
    {
      ADMAudioFileSamples *afile;
      const ADMAudioObject *obj;
    
      // attempt to find object
      if (((obj = dynamic_cast<const ADMAudioObject *>(adm->GetObjectByID(name, ADMAudioObject::Type))) != NULL) ||
          ((obj = dynamic_cast<const ADMAudioObject *>(adm->GetObjectByName(name, ADMAudioObject::Type))) != NULL))
      {
        if ((afile = new ADMAudioFileSamples(adm, file.GetSamples(), obj)) != NULL)
        {
          DEBUG2(("Adding audio object %s (%u channels, %lu samples) to list", obj->ToString().c_str(), afile->GetChannels(), afile->GetLength()));
          AddFile(afile);
          success = true;
        }
        else ERROR("Unable to duplicate file handle for '%s'", name);
      }
      else ERROR("Failed to find audio object named or with ID of '%s'", name);
    }
  }
  else ERROR("File does not have an ADM associated with it");

  return success;
}

void PlaybackEngine::Clear()
{
  ThreadLock lock(tlock);
  playlist.Clear();
}

void PlaybackEngine::Reset()
{
  ThreadLock lock(tlock);
  playlist.Reset();
}

void PlaybackEngine::SetFileChannelsAndSampleRate()
{
  SoundFileSamples *file;

  // if not at the end of the play list, try and read out samples
  if ((file = playlist.GetFile()) != NULL)
  {
    // tell renderers how many input channels to expect
    SetInputChannels(file->GetClip().nchannels);

    // update sample rate
    SetInputSampleRate(file->GetFormat()->GetSampleRate());
  }
}

/*--------------------------------------------------------------------------------*/
/** Update all positions if necessary
 */
/*--------------------------------------------------------------------------------*/
void PlaybackEngine::UpdateAllPositions(bool force)
{
  ThreadLock lock(tlock);

  // if inputchannels = 0 then renderer has changed so set up channels and sample rate again
  if (!inputchannels) SetFileChannelsAndSampleRate();

  // update positions from PositionGenerator and to renderer
  AudioPositionProcessor::UpdateAllPositions(force);
}

/*--------------------------------------------------------------------------------*/
/** Render from one set of channels to another
 *
 * @param src source buffer
 * @param dst destination buffer
 * @param nsrcchannels number channels in source buffer
 * @param ndstchannels number channels desired in destination buffer
 * @param nsrcframes number of sample frames in source buffer
 * @param ndstframes maximum number of sample frames that can be put in destination
 * @param level level to mix output to destination
 *
 * @return number of frames written to destination
 *
 * @note samples may be LOST if nsrcframes > ndstframes
 * @note ASSUMES destination is BLANKED out!
 *
 */
/*--------------------------------------------------------------------------------*/
uint_t PlaybackEngine::Render(const Sample_t *src, Sample_t *dst,
                              uint_t nsrcchannels, uint_t ndstchannels, uint_t nsrcframes, uint_t ndstframes, Sample_t level)
{
  ThreadLock lock(tlock);
  uint_t frames = 0;

  UNUSED_PARAMETER(src);
  UNUSED_PARAMETER(nsrcchannels);
  UNUSED_PARAMETER(nsrcframes);

  while (ndstframes)
  {
    ThreadLock       lock(tlock);
    SoundFileSamples *file;
    uint_t           nread = 0;

    // if not at the end of the play list, try and read out samples
    if ((file = playlist.GetFile()) != NULL)
    {
      // if inputchannels = 0 then renderer has changed so set up channels and sample rate again
      if (!inputchannels) SetFileChannelsAndSampleRate();

      // calculate maximum number of frames that can be read and read them into a temporary buffer
      nread = file->ReadSamples(&samplesbuffer[0], MIN(samplesbuffer.size() / inputchannels, ndstframes));

      // end of this file, move onto next one
      if (nread == 0)
      {
        playlist.Next();
        SetFileChannelsAndSampleRate();
        continue;
      }
    }

    // allow LESS samples to be written to output than sent to renderer (for non-unity time rendering processes)
    uint_t nwritten = AudioPositionProcessor::Render(&samplesbuffer[0], dst,
                                                     inputchannels, ndstchannels,
                                                     nread, ndstframes, level);

    // if the renderer has finished outputting, break out
    if ((nread == 0) && (nwritten == 0)) break;

    // move output pointer on by resultant number of frames
    dst        += nwritten * ndstchannels;
    ndstframes -= nwritten;
    frames     += nwritten;
  }

  // if no frames written, notify renderer that processing has finished
  if (!frames) ProcessingFinished();

  return frames;
}

BBC_AUDIOTOOLBOX_END
