#ifndef __FILE_POSITION_GENERATOR__
#define __FILE_POSITION_GENERATOR__

#include <aplibs-render/PositionGenerator.h>

BBC_AUDIOTOOLBOX_START

class Playlist;
class FilePositionGenerator : public PositionGenerator
{
public:
  FilePositionGenerator(SoundPositionRenderer *_renderer, Playlist& _playlist);
  virtual ~FilePositionGenerator();

  virtual void Process();

protected:
  Playlist& playlist;
};

BBC_AUDIOTOOLBOX_END

#endif

