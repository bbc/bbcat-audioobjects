
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_LEVEL 1
#include "PlaybackEngine.h"
#include "SoundFileWithPosition.h"

using namespace std;

BBC_AUDIOTOOLBOX_START

PlaybackEngine::PlaybackEngine() : renderer(new SoundRenderer()),
                                   receiver(NULL),
                                   channels(0),
                                   nsamples(1024),
                                   samples(new int32_t[nsamples]),
                                   reporttick(0),
                                   loop_all(false)
{
  it = list.begin();
}

PlaybackEngine::~PlaybackEngine()
{
  uint_t i;

  for (i = 0; i < list.size(); i++) {
    delete list[i];
  }
  if (samples) delete[] samples;

  if (receiver) delete receiver;
  if (renderer) delete renderer;
}

void PlaybackEngine::SetRenderer(SoundRenderer *newrenderer)
{
  SoundPositionRenderer *posrenderer;

  if (renderer) delete renderer;
  renderer = newrenderer;

  if (receiver && ((posrenderer = dynamic_cast<SoundPositionRenderer *>(renderer)) != NULL)) receiver->SetRenderer(posrenderer);
}

void PlaybackEngine::SetPositionReceiver(PositionReceiver *newreceiver)
{
  SoundPositionRenderer *posrenderer;

  if (receiver) delete receiver;
  receiver = newreceiver;

  if (receiver && ((posrenderer = dynamic_cast<SoundPositionRenderer *>(renderer)) != NULL)) receiver->SetRenderer(posrenderer);
}

void PlaybackEngine::AddFile(SoundFileSamples *file)
{
  list.push_back(file);

  if (list.size() == 1) Reset();
}

void PlaybackEngine::Reset()
{
  ThreadLock lock(tlock);
  it = list.begin();
  if (it != list.end()) (*it)->SetPosition(0);
  UpdatePositions(true);
}

bool PlaybackEngine::GenerateAudio(int32_t *output, uint_t output_channels, uint_t frames)
{
  bool done = true;

  // clear entire output
  memset(output, 0, output_channels * frames * sizeof(*output));

  while (frames) {
    ThreadLock lock(tlock);
    uint_t     nread = 0;

    // if not at the end of the play list, try and read out samples
    if ((list.size() > 0) && (it != list.end())) {
      SoundFileSamples *file = *it;

      // save number of input channels for this file
      channels = file->GetClip().nchannels;

      // tell renderer how many input channels to expect
      renderer->SetInputChannels(channels);

      // tell renderer what sample rate to expect input at
      renderer->SetInputSampleRate(file->GetFormat()->GetSampleRate());

      // calculate maximum number of frames that can be read and read them into a temporary buffer
      nread = file->ReadSamples(samples, MIN(nsamples / channels, frames));

      // end of this file, move onto next one
      if (nread == 0) {
        ++it;
        // if the end of the list has been reached and looping enabled, reset to start of list
        if (loop_all && (it == list.end())) it = list.begin();

        // ensure the new file is at the start
        if (it != list.end()) (*it)->SetPosition(0);
        continue;
      }
    }

    // allow LESS samples to be written to output than sent to renderer (for non-unity time rendering processes)
    uint_t nwritten = renderer->Render(samples, output, channels, output_channels, nread, frames);

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

void PlaybackEngine::UpdatePositions(bool initial)
{
  SoundPositionRenderer *posrenderer = dynamic_cast<SoundPositionRenderer *>(renderer);
  vector<Position> reports;
  ulong_t position = 0, samplerate = 0;

  //uncomment this to ignore position updates from audio objects
  //if (!initial) return;

  if (posrenderer) {
    ThreadLock lock(tlock);

    if ((list.size() > 0) && (it != list.end())) {
      SoundFileSamplesWithPosition *file = dynamic_cast<SoundFileSamplesWithPosition *>(*it);
            
      if (file) {
        uint32_t tick = GetTickCount();
        bool report = ((tick - reporttick) >= 1000);    // report once a second

        if (initial) {
          // configure renderer with input channels and sample format before starting to play

          // save number of input channels for this file
          channels = file->GetClip().nchannels;

          // tell renderer how many input channels to expect
          renderer->SetInputChannels(channels);

          // tell renderer what sample rate to expect input at
          renderer->SetInputSampleRate(file->GetFormat()->GetSampleRate());
        }

        if (report) {
          position   = file->GetPosition();
          samplerate = file->GetFormat()->GetSampleRate();
        }

        if (receiver)
        {
          // position receiver set, use it as source of channel positions
          receiver->Process();
        }
        else
        {
          // no position receiver set, get positions from audio objects
          const vector<PositionCursor *>& cursors = file->GetCursors();
          uint_t i, n = cursors.size();

          for (i = 0; i < n; i++) {
            const Position *pos = cursors[i]->GetPosition();

            if (pos) {
              posrenderer->UpdatePosition(i, *pos, cursors[i]->GetPositionSupplement());
              if (report) reports.push_back(*pos);
            }
          }
        }

        if (report) reporttick = tick;
      }
    }
  }

#if DEBUG_LEVEL >= 2
  if (reports.size() > 0) {
    if (samplerate > 0) {
      uint64_t subseconds = ((uint64_t)position * 10000) / samplerate;
      uint_t   seconds    = (uint_t)(subseconds / 10000);
      DEBUG("%02u:%02u:%02u.%05u:", seconds / 3600, (seconds / 60) % 60, seconds % 60, (uint_t)(subseconds % 10000));
    }

    uint_t i;
    for (i = 0; i < reports.size(); i++) {
      Position pos = reports[i];

      DEBUG("%u/%u: %s", i + 1, (uint_t)reports.size(), pos.ToString().c_str());
    }
  }
#else
  UNUSED_PARAMETER(position);
  UNUSED_PARAMETER(samplerate);
#endif
}

BBC_AUDIOTOOLBOX_END
