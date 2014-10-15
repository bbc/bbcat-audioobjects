
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_LEVEL 1
#include "FilePositionGenerator.h"
#include "SoundFileWithPosition.h"
#include "Playlist.h"

BBC_AUDIOTOOLBOX_START

FilePositionGenerator::FilePositionGenerator(PositionHandler *_handler, Playlist& _playlist) : PositionGenerator(_handler),
                                                                                               playlist(_playlist)
{
}

FilePositionGenerator::~FilePositionGenerator()
{
}

void FilePositionGenerator::Process()
{
  SoundFileSamplesWithPosition *file;

  if ((file = dynamic_cast<SoundFileSamplesWithPosition *>(playlist.GetFile())) != NULL)
  {
    const std::vector<PositionCursor *>& cursors = file->GetCursors();
    uint_t i, n = cursors.size();

    for (i = 0; i < n; i++)
    {
      PositionCursor *cursor = cursors[i];
      const Position *pos;

      if ((pos = cursor->GetPosition()) != NULL)
      {
        UpdatePosition(cursor->GetChannel(), *pos, cursor->GetPositionSupplement());
      }
    }
  }
  
  PositionGenerator::Process();
}

BBC_AUDIOTOOLBOX_END
