#ifndef __FILE_POSITION_GENERATOR__
#define __FILE_POSITION_GENERATOR__

#include <bbcat-render/PositionGenerator.h>

BBC_AUDIOTOOLBOX_START

class Playlist;

/*--------------------------------------------------------------------------------*/
/** Object to push the positions of different channels to the rest of the system
 * as time progresses through a list of playback objects 
 */
/*--------------------------------------------------------------------------------*/
class FilePositionGenerator : public PositionGenerator
{
public:
  FilePositionGenerator(PositionHandler *_handler, Playlist& _playlist);
  virtual ~FilePositionGenerator();

  virtual void Process();

protected:
  Playlist& playlist;
};

BBC_AUDIOTOOLBOX_END

#endif

