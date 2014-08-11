#ifndef __ADM_FILE_WRITE_RENDERER__
#define __ADM_FILE_WRITE_RENDERER__

#include <aplibs-render/SoundRenderer.h>

#include "ADMRIFFFile.h"

BBC_AUDIOTOOLBOX_START

class ADMFileWriteRenderer : public SoundPositionRenderer, public ADMRIFFFile
{
public:
  ADMFileWriteRenderer();
  virtual ~ADMFileWriteRenderer();

  /*--------------------------------------------------------------------------------*/
  /** Close file
   *
   * @note this may take some time because it copies sample data from a temporary file
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Close();

  /*--------------------------------------------------------------------------------*/
  /** Return desired output channels
   *
   * @return number of desired output channels (0 == no limit / not restricted)
   */
  /*--------------------------------------------------------------------------------*/
  virtual uint_t GetDesiredOutputChannels() const {return GetChannels();}

  /*--------------------------------------------------------------------------------*/
  /** Render from one set of channels to another
   *
   * @param src source buffer
   * @param dst destination buffer
   * @param nsrcchannels number channels in source buffer
   * @param ndstchannels number channels desired in destination buffer
   * @param nsrcframes number of sample frames in source buffer
   * @param ndstframes maximum number of sample frames that can be put in destination
   * @param level level to mix output to destination
   *
   * @return number of frames written to destination
   *
   * @note samples may be LOST if nsrcframes > ndstframes
   * @note ASSUMES destination is BLANKED out!
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual uint_t Render(const Sample_t *src, Sample_t *dst,
                        uint_t nsrcchannels, uint_t ndstchannels, uint_t nsrcframes, uint_t ndstframes, Sample_t level = 1.0);

  /*--------------------------------------------------------------------------------*/
  /** Render from one set of channels to another
   *
   * @param src source buffer
   * @param srcformat format of source buffer
   * @param dst destination buffer
   * @param dstformat format of destination buffer
   * @param nsrcchannels number channels in source buffer
   * @param ndstchannels number channels desired in destination buffer
   * @param nsrcframes number of sample frames in source buffer
   * @param ndstframes maximum number of sample frames that can be put in destination
   * @param level level to mix output to destination
   *
   * @return number of frames written to destination
   *
   * @note this version is a generic renderer function that can handle any format
   * @note of input and output but will have a performance and memory hit as a result!
   *
   * @note samples may be LOST if nsrcframes > ndstframes
   * @note ASSUMES destination is BLANKED out!
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual uint_t Render(const uint8_t *src, SampleFormat_t srcformat,
                        uint8_t       *dst, SampleFormat_t dstformat,
                        uint_t nsrcchannels, uint_t ndstchannels, uint_t nsrcframes, uint_t ndstframes, Sample_t level = 1.0);

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
