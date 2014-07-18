
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

PlaybackEngine::PlaybackEngine() : AudioPositionProcessor(),
                                   nsamples(1024),
                                   samples(new int32_t[nsamples])
{
  SetExplicitGenerator(new FilePositionGenerator(renderer, playlist));
}

PlaybackEngine::~PlaybackEngine()
{
  if (samples) delete[] samples;
}

/*--------------------------------------------------------------------------------*/
/** Add file to playlist
 *
 * @note object will be DELETED on destruction of this object!
 */
/*--------------------------------------------------------------------------------*/
void PlaybackEngine::AddFile(SoundFileSamples *file)
{
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
  const ADMData *adm;
  bool success = false;

  if ((adm = file.GetADM()) != NULL)
  {
    if (strcasecmp(name, "all") == 0)
    {
      ADMAudioFileSamples *afile;
    
      // play entire file
      if ((afile = new ADMAudioFileSamples(adm, file.GetSamples())) != NULL) {
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
    // save number of input channels for this file
    inputchannels = file->GetClip().nchannels;

    if (renderer)
    {
      // tell renderer how many input channels to expect
      renderer->SetInputChannels(inputchannels);
    }

    // update sample rate
    SetInputSampleRate(file->GetFormat()->GetSampleRate());
  }
}

/*--------------------------------------------------------------------------------*/
/** Update positions of channels currently being played out
 */
/*--------------------------------------------------------------------------------*/
void PlaybackEngine::UpdatePositions()
{
  ThreadLock lock(tlock);

  // if inputchannels = 0 then renderer has changed so set up channels and sample rate again
  if (!inputchannels) SetFileChannelsAndSampleRate();

  // update positions from PositionGenerator and to renderer
  AudioPositionProcessor::UpdatePositions();
}


/*--------------------------------------------------------------------------------*/
/** Generate a buffer worth of samples from list of audio files
 *
 * @param src source buffer (IGNORED)
 * @param srcformat format of source buffer (IGNORED)
 * @param dst destination buffer
 * @param dstformat format of destination buffer
 * @param nsrcchannels number channels in source buffer (IGNORED)
 * @param ndstchannels number channels desired in destination buffer
 * @param nsrcframes number of sample frames in source buffer (IGNORED)
 * @param ndstframes maximum number of sample frames that can be put in destination
 *
 * @return number of frames written to destination
 *
 * @note for any implementation of a renderer, this function is the PREFERRED one to override
 * @note whereas overriding any of the specific type versions below is DISCOURAGED
 *
 * @note samples may be LOST if nsrcframes > ndstframes
 * @note ASSUMES destination is BLANKED out!
 *
 */
/*--------------------------------------------------------------------------------*/
uint_t PlaybackEngine::Render(const uint8_t *src, SampleFormat_t srcformat,
                              uint8_t       *dst, SampleFormat_t dstformat,
                              uint_t nsrcchannels, uint_t ndstchannels, uint_t nsrcframes, uint_t ndstframes)
{
  if (renderer)
  {
    uint_t dstbps = GetBytesPerSample(dstformat);
    uint_t frames = 0;

    UNUSED_PARAMETER(src);
    UNUSED_PARAMETER(srcformat);
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
        nread = file->ReadSamples(samples, MIN(nsamples / inputchannels, ndstframes));

        // end of this file, move onto next one
        if (nread == 0) {
          playlist.Next();
          SetFileChannelsAndSampleRate();
          continue;
        }
      }

      // allow LESS samples to be written to output than sent to renderer (for non-unity time rendering processes)
      uint_t nwritten = renderer->Render((const uint8_t *)samples,
                                         SampleFormatOf(samples),
                                         dst, dstformat,
                                         inputchannels, ndstchannels,
                                         nread, ndstframes);

      // if the renderer has finished outputting, break out
      if ((nread == 0) && (nwritten == 0)) break;

      // move output pointer on by resultant number of frames
      dst        += nwritten * ndstchannels * dstbps;
      ndstframes -= nwritten;
      frames     += nwritten;
    }

    // if no frames written, notify renderer that processing has finished
    if (!frames) renderer->ProcessingFinished();

    return frames;
  }

  // no renderer, use default
  return defaultrenderer.Render(src, srcformat,
                                dst, dstformat,
                                nsrcchannels, ndstchannels,
                                nsrcframes,   ndstframes);
}

BBC_AUDIOTOOLBOX_END
