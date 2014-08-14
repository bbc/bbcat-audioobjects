
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DEBUG_LEVEL 2

#include "ADMFileWriteRenderer.h"

BBC_AUDIOTOOLBOX_START

ADMFileWriteRenderer::ADMFileWriteRenderer() : SoundPositionRenderer()
{
}

ADMFileWriteRenderer::~ADMFileWriteRenderer()
{
  Close();
}

/*--------------------------------------------------------------------------------*/
/** Close file
 *
 * @note this may take some time because it copies sample data from a temporary file
 */
/*--------------------------------------------------------------------------------*/
void ADMFileWriteRenderer::Close()
{
  uint64_t t = filesamples ? filesamples->GetPositionNS() : 0;
  uint_t i;

  for (i = 0; i < cursors.size(); i++)
  {
    cursors[i]->Seek(t);
    cursors[i]->EndPositionChanges();
    delete cursors[i];
  }

  ADMRIFFFile::Close();

#if DEBUG_LEVEL >= 2
  {
    std::string str;
    adm->Dump(str);
    DEBUG("ADM:\n%s", str.c_str());
  }
#endif
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
 * @param level level to mix output to destination
 *
 * @return number of frames written to destination
 *
 * @note samples may be LOST if nsrcframes > ndstframes
 * @note ASSUMES destination is BLANKED out!
 *
 */
/*--------------------------------------------------------------------------------*/
uint_t ADMFileWriteRenderer::Render(const Sample_t *src, Sample_t *dst,
                                    uint_t nsrcchannels, uint_t ndstchannels, uint_t nsrcframes, uint_t ndstframes, Sample_t level)
{
  UNUSED_PARAMETER(dst);
  UNUSED_PARAMETER(ndstchannels);
  UNUSED_PARAMETER(level);

  if (filesamples)
  {
    // write to file
    filesamples->WriteSamples(src, 0, nsrcchannels, nsrcframes);
  }

  return MIN(nsrcframes, ndstframes);
}

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
uint_t ADMFileWriteRenderer::Render(const uint8_t *src, SampleFormat_t srcformat,
                                    uint8_t       *dst, SampleFormat_t dstformat,
                                    uint_t nsrcchannels, uint_t ndstchannels, uint_t nsrcframes, uint_t ndstframes, Sample_t level)
{
  UNUSED_PARAMETER(dst);
  UNUSED_PARAMETER(dstformat);
  UNUSED_PARAMETER(ndstchannels);
  UNUSED_PARAMETER(level);

  if (filesamples)
  {
    // write to file
    filesamples->WriteSamples(src, srcformat, nsrcframes, 0, nsrcchannels);
  }

  return MIN(nsrcframes, ndstframes);
}

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
void ADMFileWriteRenderer::UpdatePositionEx(uint_t channel, const Position& pos, const ParameterSet *supplement)
{
  if (cursors.size() == 0) GetADM()->CreateCursors(cursors);

  if (channel < cursors.size())
  {
    PositionCursor *cursor = cursors[channel];

    if (filesamples) cursor->Seek(filesamples->GetPositionNS());
    cursor->SetPosition(pos, supplement);
  }
}

BBC_AUDIOTOOLBOX_END
