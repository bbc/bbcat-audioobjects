
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_LEVEL 1
#include "Playlist.h"

#include "SoundFileAttributes.h"

using namespace std;

BBC_AUDIOTOOLBOX_START

Playlist::Playlist() : loop_all(false)
{
  it = list.begin();
}

Playlist::~Playlist()
{
  uint_t i;

  for (i = 0; i < list.size(); i++) {
    delete list[i];
  }
}

/*--------------------------------------------------------------------------------*/
/** Add file to list
 *
 * @note object will be DELETED on destruction of this object!
 */
/*--------------------------------------------------------------------------------*/
void Playlist::AddFile(SoundFileSamples *file)
{
  list.push_back(file);

  if (list.size() == 1) Reset();
}

/*--------------------------------------------------------------------------------*/
/** Reset to start of playback list
 */
/*--------------------------------------------------------------------------------*/
void Playlist::Reset()
{
  it = list.begin();
  if (it != list.end()) (*it)->SetPosition(0);
}

/*--------------------------------------------------------------------------------*/
/** Move onto next file (or stop if looping is disabled)
 */
/*--------------------------------------------------------------------------------*/
void Playlist::Next()
{
  if (it != list.end())
  {
    // advance along list and reset position of file if not at end of list
    if ((++it) != list.end()) (*it)->SetPosition(0);
    // else if looping enabled then reset to the start of the list
    else if (loop_all) Reset();
  }
}

/*--------------------------------------------------------------------------------*/
/** Return current file or NULL if the end of the list has been reached
 */
/*--------------------------------------------------------------------------------*/
SoundFileSamples *Playlist::GetFile()
{
  return (it != list.end()) ? *it : NULL;
}

BBC_AUDIOTOOLBOX_END
