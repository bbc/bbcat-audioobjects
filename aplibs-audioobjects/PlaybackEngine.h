#ifndef __PLAYBACK_ENGINE__
#define __PLAYBACK_ENGINE__

#include <vector>

#include <aplibs-dsp/ThreadLock.h>
#include <aplibs-render/SoundRenderer.h>

#include "SoundFileAttributes.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Object to play out a list of audio files, updating the positions to the
 * specified renderer
 */
/*--------------------------------------------------------------------------------*/
class PlaybackEngine {
public:
  PlaybackEngine();
  virtual ~PlaybackEngine();

  /*--------------------------------------------------------------------------------*/
  /** Add file to list
   *
   * @note object will be DELETED on destruction of this object!
   */
  /*--------------------------------------------------------------------------------*/
  virtual void AddFile(SoundFileSamples *file);

  /*--------------------------------------------------------------------------------*/
  /** Set renderer
   *
   * @note object will be DELETED on destruction of this object!
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetRenderer(SoundRenderer *newrenderer);

  /*--------------------------------------------------------------------------------*/
  /** Get current renderer
   */
  /*--------------------------------------------------------------------------------*/
  SoundRenderer *GetRenderer() {return renderer;}

  /*--------------------------------------------------------------------------------*/
  /** Enable/disable looping
   */
  /*--------------------------------------------------------------------------------*/
  virtual void EnableLoop(bool enable = true) {loop_all = enable;}

  /*--------------------------------------------------------------------------------*/
  /** Reset to start of playback list
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Reset();

  /*--------------------------------------------------------------------------------*/
  /** Update positions of channels currently being played out
   *
   * @param initial true if this is the initial call
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdatePositions(bool initial = false);

  /*--------------------------------------------------------------------------------*/
  /** Generate a buffer worth of samples from list of audio files
   *
   * @param output buffer to write audio to
   * @param output_channels expected width of output buffer
   * @param frames number of frames to generate
   *
   * @return true if all of or part of buffer written
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool GenerateAudio(int32_t *output, uint_t output_channels, uint_t frames);

protected:
  ThreadLockObject  tlock;
  SoundRenderer     *renderer;
  uint_t            channels;
  uint_t            nsamples;
  int32_t           *samples;
  uint32_t          reporttick;
  bool              loop_all;
  std::vector<SoundFileSamples *> list;
  std::vector<SoundFileSamples *>::iterator it;
};

BBC_AUDIOTOOLBOX_END

#endif
