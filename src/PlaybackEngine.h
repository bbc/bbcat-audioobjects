#ifndef __PLAYBACK_ENGINE__
#define __PLAYBACK_ENGINE__

#include <vector>

#include <bbcat-base/ThreadLock.h>
#include <bbcat-render/SoundRenderer.h>
#include <bbcat-render/AudioPositionProcessor.h>

#include "Playlist.h"
#include "SoundFileAttributes.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Object to play out a list of audio files, updating the positions to the
 * specified renderer
 */
/*--------------------------------------------------------------------------------*/
class ADMRIFFFile;
class PlaybackEngine : public AudioPositionProcessor
{
  CONTROL_RECEIVER();
public:
  PlaybackEngine();
  virtual ~PlaybackEngine();

  /*--------------------------------------------------------------------------------*/
  /** Set parameters within object
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetParameters(const ParameterSet& parameters);

  /*--------------------------------------------------------------------------------*/
  /** Get a list of controls for this object
   */
  /*--------------------------------------------------------------------------------*/
  static void GetControlDescriptions(std::vector<const PARAMETERDESC *>& list);

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
  /** Return whether playlist is empty
   */
  /*--------------------------------------------------------------------------------*/
  bool Empty();

  /*--------------------------------------------------------------------------------*/
  /** Clear play list
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Clear();

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
  /** Update all positions if necessary
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdateAllPositions(bool force = false);

  /*--------------------------------------------------------------------------------*/
  /** Render from one set of channels to another
   *
   * @param src source buffer
   * @param dst destination buffer
   * @param nsrcchannels number channels in source buffer
   * @param ndstchannels number channels desired in destination buffer
   * @param nsrcframes number of sample frames in source buffer
   * @param ndstframes maximum number of sample frames that can be put in destination
   *
   * @return number of frames written to destination
   *
   * @note samples may be LOST if nsrcframes > ndstframes
   * @note ASSUMES destination is BLANKED out!
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual uint_t Render(const Sample_t *src, Sample_t *dst,
                        uint_t nsrcchannels, uint_t ndstchannels, uint_t nsrcframes, uint_t ndstframes);

  /*--------------------------------------------------------------------------------*/
  /** Return max number of audio channels of playlist
   */
  /*--------------------------------------------------------------------------------*/
  uint_t GetMaxOutputChannels() const {return playlist.GetMaxOutputChannels();}

protected:
  /*--------------------------------------------------------------------------------*/
  /** Register a self-registering-parametric-object or return -1
   *
   * @return index (if applicable) or -1 for unrecognized type
   */
  /*--------------------------------------------------------------------------------*/
  virtual int Register(SelfRegisteringParametricObject *obj, const ParameterSet& parameters);

  /*--------------------------------------------------------------------------------*/
  /** Set an arbitrary control within this object to a value
   *
   * @param handler source of control change
   * @param value new value of control
   *
   * @return true if successful
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool SetLocalControl(ControlHandler *handler, const ParameterSet& value);

  virtual void SetFileChannelsAndSampleRate();

protected:
  Playlist              playlist;
  std::vector<Sample_t> samplesbuffer;
};

BBC_AUDIOTOOLBOX_END

#endif
