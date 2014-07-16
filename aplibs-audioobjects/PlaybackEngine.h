#ifndef __PLAYBACK_ENGINE__
#define __PLAYBACK_ENGINE__

#include <vector>

#include <aplibs-dsp/ThreadLock.h>
#include <aplibs-render/SoundRenderer.h>
#include <aplibs-render/AudioPositionProcessor.h>

#include "Playlist.h"
#include "SoundFileAttributes.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Object to play out a list of audio files, updating the positions to the
 * specified renderer
 */
/*--------------------------------------------------------------------------------*/
class ADMRIFFFile;
class PlaybackEngine : public AudioPositionProcessor {
public:
  PlaybackEngine();
  virtual ~PlaybackEngine();

  /*--------------------------------------------------------------------------------*/
  /** Add file to playlist
   *
   * @param file COPY of file to add 
   *
   * @note object will be DELETED on destruction of this object!
   */
  /*--------------------------------------------------------------------------------*/
  virtual void AddFile(SoundFileSamples *file);

  /*--------------------------------------------------------------------------------*/
  /** Add audio object to playlist
   *
   * @param file open audio file support audio objects
   * @param name name of audio object or 'all' for entire file
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool AddObject(const ADMRIFFFile& file, const char *name);

  /*--------------------------------------------------------------------------------*/
  /** Enable/disable looping
   */
  /*--------------------------------------------------------------------------------*/
  virtual void EnableLoop(bool enable = true) {playlist.EnableLoop(enable);}

  /*--------------------------------------------------------------------------------*/
  /** Reset to start of playback list
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Reset();

  /*--------------------------------------------------------------------------------*/
  /** Update positions of channels currently being played out
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdatePositions();

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
  virtual bool ProcessAudio(const sint32_t *input, uint_t input_channels, sint32_t *output, uint_t output_channels, uint_t frames);

protected:
  virtual void SetFileChannelsAndSampleRate();

protected:
  ThreadLockObject  tlock;
  Playlist          playlist;
  uint_t            nsamples;
  int32_t           *samples;
  uint32_t          reporttick;
};

BBC_AUDIOTOOLBOX_END

#endif
