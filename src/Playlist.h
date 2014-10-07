#ifndef __SIMPLE_PLAYLIST__
#define __SIMPLE_PLAYLIST__

#include <vector>

#include <bbcat-base/misc.h>

BBC_AUDIOTOOLBOX_START

class SoundFileSamples;
class Playlist
{
public:
  Playlist();
  ~Playlist();

  /*--------------------------------------------------------------------------------*/
  /** Return whether the playlist is empty
   */
  /*--------------------------------------------------------------------------------*/
  bool Empty() const {return (list.size() == 0);}

  /*--------------------------------------------------------------------------------*/
  /** Add file to list
   *
   * @note object will be DELETED on destruction of this object!
   */
  /*--------------------------------------------------------------------------------*/
  void AddFile(SoundFileSamples *file);

  /*--------------------------------------------------------------------------------*/
  /** Clear playlist
   */
  /*--------------------------------------------------------------------------------*/
  void Clear();

  /*--------------------------------------------------------------------------------*/
  /** Enable/disable looping
   */
  /*--------------------------------------------------------------------------------*/
  void EnableLoop(bool enable = true) {loop_all = enable;}

  /*--------------------------------------------------------------------------------*/
  /** Reset to start of playback list
   */
  /*--------------------------------------------------------------------------------*/
  void Reset();

  /*--------------------------------------------------------------------------------*/
  /** Move onto next file (or stop if looping is disabled)
   */
  /*--------------------------------------------------------------------------------*/
  void Next();

  /*--------------------------------------------------------------------------------*/
  /** Return current file or NULL if the end of the list has been reached
   */
  /*--------------------------------------------------------------------------------*/
  SoundFileSamples *GetFile();

protected:
  std::vector<SoundFileSamples *> list;
  std::vector<SoundFileSamples *>::iterator it;
  bool loop_all;
};

BBC_AUDIOTOOLBOX_END

#endif
