
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_LEVEL 1
#include "PlaybackEngine.h"
#include "SoundFileWithPosition.h"
#include "FilePositionGenerator.h"
#include "ADMRIFFFile.h"
#include "ADMAudioFileSamples.h"

using namespace std;

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
 * @note object will be DELETED on destruction of this object!
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
    SetSampleRate(file->GetFormat()->GetSampleRate());
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

  // update positions from PositionGenerator
  if (generator) generator->Process();
}

/*--------------------------------------------------------------------------------*/
/** Generate a buffer worth of samples from list of audio files
 *
 * @param input buffer to read audio from
 * @param input_channels expected width of input buffer
 * @param output buffer to write audio to
 * @param output_channels expected width of output buffer
 * @param frames number of frames to generate
 *
 * @return true if all of or part of buffer written
 */
/*--------------------------------------------------------------------------------*/
bool PlaybackEngine::ProcessAudio(const sint32_t *input, uint_t input_channels, sint32_t *output, uint_t output_channels, uint_t frames)
{
  bool done = true;

  UNUSED_PARAMETER(input);
  UNUSED_PARAMETER(input_channels);

  // clear entire output
  memset(output, 0, output_channels * frames * sizeof(*output));

  while (frames)
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
      nread = file->ReadSamples(samples, MIN(nsamples / inputchannels, frames));

      // end of this file, move onto next one
      if (nread == 0) {
        playlist.Next();
        SetFileChannelsAndSampleRate();
        continue;
      }
    }

    // allow LESS samples to be written to output than sent to renderer (for non-unity time rendering processes)
    uint_t nwritten = renderer->Render(samples, output, inputchannels, output_channels, nread, frames);

    // if the renderer has finished outputting, break out
    if ((nread == 0) && (nwritten == 0)) break;

    // move output pointer on by resultant number of frames
    output += nwritten * output_channels;
    frames -= nwritten;

    done = false;
  }

  if (done) renderer->ProcessingFinished();

  return !done;
}

BBC_AUDIOTOOLBOX_END
