#ifndef __ADM_FILE_WRITE_RENDERER__
#define __ADM_FILE_WRITE_RENDERER__

#include <bbcat-render/SoundConsumer.h>

#include "ADMRIFFFile.h"

BBC_AUDIOTOOLBOX_START

class ADMFileWriter : public SoundPositionConsumer, public ADMRIFFFile
{
  SELF_REGISTER_CREATOR(ADMFileWriter);

public:
  ADMFileWriter();
  virtual ~ADMFileWriter();

  /*--------------------------------------------------------------------------------*/
  /** Create a WAVE/RIFF file
   *
   * @param filename filename of file to create
   * @param samplerate sample rate of audio
   * @param nchannels number of audio channels
   * @param format sample format of audio in file
   *
   * @return true if file created properly
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool Create(const char *filename, uint32_t samplerate = 48000, uint_t nchannels = 2, SampleFormat_t format = SampleFormat_24bit);

  /*--------------------------------------------------------------------------------*/
  /** Close file
   *
   * @note this may take some time because it copies sample data from a temporary file
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Close();

  /*--------------------------------------------------------------------------------*/
  /** Create ADM from text file
   *
   * @param filename text filename (see below for format)
   *
   * @return true if successful
   *
   * The file MUST be of the following format with each entry on its own line:
   * <ADM programme name>[:<ADM content name>]
   *
   * then for each track:
   * <track>:<trackname>:<objectname>
   *
   * Where <track> is 1..number of tracks available within ADM
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool CreateADM(const char *filename);

  /*--------------------------------------------------------------------------------*/
  /** Return desired output channels
   *
   * @return number of desired output channels (0 == no limit / not restricted)
   */
  /*--------------------------------------------------------------------------------*/
  virtual uint_t GetDesiredOutputChannels() const {return GetChannels();}

  /*--------------------------------------------------------------------------------*/
  /** Consume audio
   *
   * @param src source buffer
   * @param nsrcchannels number channels in source buffer
   * @param nsrcframes number of sample frames in source buffer
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Consume(const Sample_t *src, uint_t nsrcchannels, uint_t nsrcframes);

  /*--------------------------------------------------------------------------------*/
  /** Consume audio
   *
   * @param src source buffer
   * @param srcformat format of source buffer
   * @param nsrcchannels number channels in source buffer
   * @param nsrcframes number of sample frames in source buffer
   *
   * @note this is just a wrapper for the above but derived classes can either override this or the above
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Consume(const uint8_t *src, SampleFormat_t srcformat, uint_t nsrcchannels, uint_t nsrcframes);

protected:
  /*--------------------------------------------------------------------------------*/
  /** Overridable update position function 
   *
   * @param channel channel to change the position of
   * @param pos new position
   * @param supplement optional extra information
   *
   * @note this is the function that should be overridden in derived objects
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdatePositionEx(uint_t channel, const Position& pos, const ParameterSet *supplement);

protected:
  std::vector<PositionCursor *> cursors;
};

BBC_AUDIOTOOLBOX_END

#endif
