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
  virtual uint_t Render(const uint8_t *src, SampleFormat_t srcformat,
                        uint8_t       *dst, SampleFormat_t dstformat,
                        uint_t nsrcchannels, uint_t ndstchannels, uint_t nsrcframes, uint_t ndstframes);

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
