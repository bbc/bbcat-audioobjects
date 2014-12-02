
#include <string.h>

#define DEBUG_LEVEL 2
#include <bbcat-render/SelfRegisteringControlReceiver.h>
#include "PlaybackEngine.h"
#include "SoundFileWithPosition.h"
#include "FilePositionGenerator.h"
#include "ADMFileReader.h"
#include "ADMAudioFileSamples.h"

BBC_AUDIOTOOLBOX_START

SELF_REGISTERING_CONTROL_RECEIVER(PlaybackEngine, "processor");

static const struct {
  PARAMETERDESC loop;
} _controls = 
{
  {"loop", "Enable looping of files"},
};

PlaybackEngine::PlaybackEngine() : AudioPositionProcessor()
{
  samplesbuffer.resize(4096);
}

PlaybackEngine::PlaybackEngine(const ParameterSet& parameters) : AudioPositionProcessor(parameters)
{
  samplesbuffer.resize(4096);

  SetParameters(parameters);
}

PlaybackEngine::~PlaybackEngine()
{
}

/*--------------------------------------------------------------------------------*/
/** Set parameters within object (*only* parameters that can be set more than once)
 */
/*--------------------------------------------------------------------------------*/
void PlaybackEngine::SetParameters(const ParameterSet& parameters)
{
  AudioPositionProcessor::SetParameters(parameters);

  bool _loop;
  if (parameters.Get(_controls.loop.name, _loop)) EnableLoop(_loop);
}

/*--------------------------------------------------------------------------------*/
/** Get a list of controls for this object
 */
/*--------------------------------------------------------------------------------*/
void PlaybackEngine::GetControlDescriptions(std::vector<const PARAMETERDESC *>& list)
{
  const PARAMETERDESC *pcontrols = (const PARAMETERDESC *)&_controls;
  uint_t i, n = sizeof(_controls) / sizeof(pcontrols[0]);

  AudioPositionProcessor::GetControlDescriptions(list);

  for (i = 0; i < n; i++) list.push_back(pcontrols + i);
}

/*--------------------------------------------------------------------------------*/
/** Set an arbitrary control within this object to a value
 *
 * @param handler source of control change
 * @param value new value of control
 *
 * @return true if successful
 */
/*--------------------------------------------------------------------------------*/
bool PlaybackEngine::SetLocalControl(ControlHandler *handler, const ParameterSet& value)
{
  std::string control;
  bool success = false;

  UNUSED_PARAMETER(handler);

  if (value.Get(ControlReceiver::controlname, control))
  {
    int ival;

    if ((control == _controls.loop.name) && value.Get(ControlReceiver::valuename, ival))
    {
      EnableLoop(ival);
      success = true;
    }
  }

  if (!success) success = AudioPositionProcessor::SetLocalControl(handler, value);

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Return the number of capture channels required: NOT necessarily the same as input channels
 */
/*--------------------------------------------------------------------------------*/
uint_t PlaybackEngine::GetCaptureChannels() const
{
  ThreadLock lock(tlock);
  
  // if there is a playlist there is NO requirement for capture channels!
  return Empty() ? inputchannels : 0;
}

/*--------------------------------------------------------------------------------*/
/** Add file to playlist
 *
 * @note object will be DELETED on destruction of this object!
 */
/*--------------------------------------------------------------------------------*/
void PlaybackEngine::AddFile(SoundFileSamples *file)
{
  ThreadLock lock(tlock);
  bool empty = playlist.Empty();

  playlist.AddFile(file);

  if (empty)
  {
    // first file added, set up renderer
    SetFileChannelsAndSampleRate();
    // configure positions to come from playlist
    SetGenerator(new FilePositionGenerator(this, playlist));
  }
}

/*--------------------------------------------------------------------------------*/
/** Add audio object to playlist
 *
 * @param file open audio file support audio objects
 * @param name name of audio object or 'all' for entire file
 */
/*--------------------------------------------------------------------------------*/
bool PlaybackEngine::AddObject(const ADMRIFFFile& file, const char *name)
{
  ThreadLock lock(tlock);
  bool success = false;

  if (file.IsOpen())
  {
    const ADMData *adm;

    if ((adm = file.GetADM()) != NULL)
    {
      ADMAudioFileSamples *afile;

      if (strcasecmp(name, "all") == 0)
      {
        std::vector<const ADMAudioObject *> objects;

        // get list of ADMAudioObjects
        adm->GetAudioObjectList(objects);

        // play entire file
        if ((afile = new ADMAudioFileSamples(file.GetSamples())) != NULL)
        {
          // add all objects to playback
          afile->Add(objects);

          DEBUG2(("Adding entire audio file (%u channels, %lu samples) to list", afile->GetChannels(), (ulong_t)afile->GetSampleLength()));
          AddFile(afile);
          success = true;
        }
        else ERROR("Unable to duplicate file handle for '%s'", name);
      }
      else
      {
        const ADMAudioObject *obj;
    
        // attempt to find object
        if (((obj = dynamic_cast<const ADMAudioObject *>(adm->GetObjectByID(name, ADMAudioObject::Type))) != NULL) ||
            ((obj = dynamic_cast<const ADMAudioObject *>(adm->GetObjectByName(name, ADMAudioObject::Type))) != NULL))
        {
          if ((afile = new ADMAudioFileSamples(file.GetSamples(), obj)) != NULL)
          {
            DEBUG2(("Adding audio object %s (%u channels, %lu samples) to list", obj->ToString().c_str(), afile->GetChannels(), (ulong_t)afile->GetSampleLength()));
            AddFile(afile);
            success = true;
          }
          else ERROR("Unable to duplicate file handle for '%s'", name);
        }
        else ERROR("Failed to find audio object named or with ID of '%s'", name);
      }
    }
    else ERROR("File does not have an ADM associated with it");
  }
  else ERROR("File is not open");

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Return whether playlist is empty
 */
/*--------------------------------------------------------------------------------*/
bool PlaybackEngine::Empty() const
{
  ThreadLock lock(tlock);
  return playlist.Empty();
}

/*--------------------------------------------------------------------------------*/
/** Clear play list
 */
/*--------------------------------------------------------------------------------*/
void PlaybackEngine::Clear()
{
  ThreadLock lock(tlock);
  // clear playlist
  playlist.Clear();
  // reset inputchannels (so that the renderers get set up again with the correct number of input channels)
  inputchannels = 0;
  // remove file based position source
  SetGenerator(NULL);
}

/*--------------------------------------------------------------------------------*/
/** Reset to start of playback list
 */
/*--------------------------------------------------------------------------------*/
void PlaybackEngine::Reset()
{
  ThreadLock lock(tlock);
  playlist.Reset();
}

void PlaybackEngine::SetFileChannelsAndSampleRate()
{
  SoundFileSamples *file;

  // if not at the end of the play list, try and read out samples
  if ((file = playlist.GetFile()) != NULL)
  {
    // tell renderers how many input channels to expect
    SetInputChannels(file->GetClip().nchannels);

    // update sample rate
    SetInputSampleRate(file->GetFormat()->GetSampleRate());
  }
}

/*--------------------------------------------------------------------------------*/
/** Register a self-registering-parametric-object or return -1
 *
 * @param obj object created by the above
 * @param parameters the set of parameters used to create the object with
 *
 * @return index that object was registered using (may be global or local to a category) or -1 for failure
 */
/*--------------------------------------------------------------------------------*/
int PlaybackEngine::Register(SelfRegisteringParametricObject *obj, const ParameterSet& parameters)
{
  ADMFileReader *reader;
  int index = -1;

  if ((reader = dynamic_cast<ADMFileReader *>(obj)) != NULL)
  {
    // ADMFileReader object will have read the desired object ADM object from the parameters
    if (AddObject(*reader, reader->GetADMObject().c_str())) index = playlist.GetCount();
  }
  else index = AudioPositionProcessor::Register(obj, parameters);

  return index;
}

/*--------------------------------------------------------------------------------*/
/** Update all positions if necessary
 */
/*--------------------------------------------------------------------------------*/
void PlaybackEngine::UpdateAllPositions(bool force)
{
  ThreadLock lock(tlock);

  // if inputchannels = 0 then renderer has changed so set up channels and sample rate again
  if (!inputchannels) SetFileChannelsAndSampleRate();

  // update positions from PositionGenerator and to renderer
  AudioPositionProcessor::UpdateAllPositions(force);
}

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
 * @note audio will be ADDED to the destination
 *
 */
/*--------------------------------------------------------------------------------*/
uint_t PlaybackEngine::Render(const Sample_t *src, Sample_t *dst,
                              uint_t nsrcchannels, uint_t ndstchannels, uint_t nsrcframes, uint_t ndstframes)
{
  ThreadLock lock(tlock);
  uint_t frames = 0;

  if (playlist.Empty())
  {
    // abort if there's no audio input
    if (nsrcchannels)
    {
      // no files to play, revert to processing input
      frames = AudioPositionProcessor::Render(src, dst, nsrcchannels, ndstchannels, nsrcframes, ndstframes);
    }
    //else ERROR("No audio source data for renderer");
  }
  else
  {
    while (ndstframes)
    {
      ThreadLock       lock(tlock);
      SoundFileSamples *file;
      uint_t           nread = 0;

      // if not at the end of the play list, try and read out samples
      if ((file = playlist.GetFile()) != NULL)
      {
        // if inputchannels = 0 then renderer has changed so set up channels and sample rate again
        if (!inputchannels) SetFileChannelsAndSampleRate();

        // calculate maximum number of frames that can be read and read them into a temporary buffer
        nread = file->ReadSamples(&samplesbuffer[0], 0, inputchannels, MIN(samplesbuffer.size() / inputchannels, ndstframes));

        // end of this file, move onto next one
        if (nread == 0)
        {
          playlist.Next();
          SetFileChannelsAndSampleRate();
          continue;
        }
      }

      // allow LESS samples to be written to output than sent to renderer (for non-unity time rendering processes)
      uint_t nwritten = AudioPositionProcessor::Render(&samplesbuffer[0], dst,
                                                       inputchannels, ndstchannels,
                                                       nread, ndstframes);

      // if the renderer has finished outputting, break out
      if ((nread == 0) && (nwritten == 0)) break;

      // move output pointer on by resultant number of frames
      dst        += nwritten * ndstchannels;
      ndstframes -= nwritten;
      frames     += nwritten;
    }
  }

  // if no frames written, notify renderer that processing has finished
  if (!frames) ProcessingFinished();

  return frames;
}

BBC_AUDIOTOOLBOX_END
