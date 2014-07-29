#ifndef __RIFF_FILE__
#define __RIFF_FILE__

#include <vector>
#include <map>

#include <aplibs-dsp/misc.h>

#include "RIFFChunks.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** A WAVE/RIFF file handler
 * 
 * Provides the same functionality as most classes of the same type but ascts as a
 * base class for ADMRIFFFile which supports the ADM for audio objects
 */
/*--------------------------------------------------------------------------------*/
class RIFFFile
{
public:
  RIFFFile();
  virtual ~RIFFFile();

  /*--------------------------------------------------------------------------------*/
  /** Open a WAVE/RIFF file
   *
   * @param filename filename of file to open
   *
   * @return true if file opened and interpreted correctly (including any extra chunks if present)
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool Open(const char *filename);

  /*--------------------------------------------------------------------------------*/
  /** Return whether a file is open
   *
   * @return true if file is open
   */
  /*--------------------------------------------------------------------------------*/
  bool IsOpen() const {return (file && (file->isopen()));}
  virtual void Close();

  /*--------------------------------------------------------------------------------*/
  /** Return the underlying file object
   *
   * @return SoundFile object (or NULL)
   */
  /*--------------------------------------------------------------------------------*/
  SoundFile *GetFile() {return file;}

  /// file type enumeration
  enum
  {
    FileType_Unknown = 0,
    FileType_WAV,
    FileType_AIFF,
  };

  /*--------------------------------------------------------------------------------*/
  /** Return file type
   *
   * @return FileType_xxx enumeration of file type
   */
  /*--------------------------------------------------------------------------------*/
  uint8_t GetFileType() const {return filetype;}

  /*--------------------------------------------------------------------------------*/
  /** Return sample rate of file, if open
   *
   * @return sample rate of file or 0 if no file open
   */
  /*--------------------------------------------------------------------------------*/
  uint32_t GetSampleRate() const {return fileformat ? fileformat->GetSampleRate() : 0;}

  /*--------------------------------------------------------------------------------*/
  /** Return number of channels in the file
   *
   * @return number of channels or 0 if no file open
   */
  /*--------------------------------------------------------------------------------*/
  uint_t GetChannels() const {return fileformat ? fileformat->GetChannels() : 0;}

  /*--------------------------------------------------------------------------------*/
  /** Return bytes per sample for data in the file
   *
   * @return bytes per sample
   */
  /*--------------------------------------------------------------------------------*/
  uint_t GetBytesPerSample() const {return fileformat ? fileformat->GetBytesPerSample() : 0;}

  /*--------------------------------------------------------------------------------*/
  /** Return sample format of samples in the file
   *
   * @return SampleFormat_xxx enumeration
   */
  /*--------------------------------------------------------------------------------*/
  SampleFormat_t GetSampleFormat() const {return fileformat ? fileformat->GetSampleFormat() : SampleFormat_Unknown;}

  /*--------------------------------------------------------------------------------*/
  /** Return sample format of samples in the file
   *
   * @return SampleFormat_xxx enumeration
   */
  /*--------------------------------------------------------------------------------*/
  ulong_t GetPosition() const {return filesamples ? filesamples->GetPosition() : 0;}

  /*--------------------------------------------------------------------------------*/
  /** Return sample format of samples in the file
   *
   * @return SampleFormat_xxx enumeration
   */
  /*--------------------------------------------------------------------------------*/
  ulong_t GetLength() const {return filesamples ? filesamples->GetLength() : 0;}

  /*--------------------------------------------------------------------------------*/
  /** Set position within sample data of file
   *
   * @param pos sample position
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetPosition(ulong_t pos) {if (filesamples) {filesamples->SetPosition(pos); UpdateSamplePosition();}}

  /*--------------------------------------------------------------------------------*/
  /** Return number of chunks found in file
   *
   * @return chunk count
   */
  /*--------------------------------------------------------------------------------*/
  uint_t GetChunkCount() const {return chunklist.size();}

  /*--------------------------------------------------------------------------------*/
  /** Return chunk at specifiec index
   *
   * @param index chunk index 0 .. number of chunks returned above
   *
   * @return pointer to RIFFChunk object
   */
  /*--------------------------------------------------------------------------------*/
  RIFFChunk *GetChunkIndex(uint_t index) {return chunklist[index];}

  /*--------------------------------------------------------------------------------*/
  /** Return chunk specified by chunk ID
   *
   * @param id 32-bit representation of chunk name (big endian)
   *
   * @return pointer to RIFFChunk object
   */
  /*--------------------------------------------------------------------------------*/
  RIFFChunk *GetChunk(uint32_t id) {return chunkmap[id];}

  /*--------------------------------------------------------------------------------*/
  /** Return chunk specified by chunk ID
   *
   * @param name chunk type name as a 4 character string
   *
   * @return pointer to RIFFChunk object
   */
  /*--------------------------------------------------------------------------------*/
  RIFFChunk *GetChunk(const char *name) {return GetChunk(IFFID(name));}

  /*--------------------------------------------------------------------------------*/
  /** Return SoundFileSamples object of file
   *
   * @return pointer to SoundFileSamples object
   */
  /*--------------------------------------------------------------------------------*/
  SoundFileSamples *GetSamples() const {return filesamples;}

  /*--------------------------------------------------------------------------------*/
  /** Read sample frames
   *
   * @param buffer destination buffer
   * @param type desired sample buffer format
   * @param nframes number of sample frames to read
   *
   * @note all channels must be read
   *
   * @return number of frames read or -1 for an error (no open file for example)
   */
  /*--------------------------------------------------------------------------------*/
  sint_t ReadFrames(uint8_t *buffer, SampleFormat_t type, uint_t nframes) {return filesamples ? filesamples->ReadSamples((uint8_t *)buffer, type, nframes) : -1;}
  sint_t ReadFrames(int16_t *buffer, uint_t nframes = 1) {return ReadFrames((uint8_t *)buffer, SampleFormatOf(buffer), nframes);}
  sint_t ReadFrames(int32_t *buffer, uint_t nframes = 1) {return ReadFrames((uint8_t *)buffer, SampleFormatOf(buffer), nframes);}
  sint_t ReadFrames(float   *buffer, uint_t nframes = 1) {return ReadFrames((uint8_t *)buffer, SampleFormatOf(buffer), nframes);}
  sint_t ReadFrames(double  *buffer, uint_t nframes = 1) {return ReadFrames((uint8_t *)buffer, SampleFormatOf(buffer), nframes);}

protected:
  /*--------------------------------------------------------------------------------*/
  /** Read as many chunks as possible
   *
   * @param maxlength maximum number of bytes to read or skip over
   *
   * @return true if read chunks processed correctly
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool ReadChunks(ulong_t maxlength);

  /*--------------------------------------------------------------------------------*/
  /** Process a chunk (overridable)
   *
   * @param chunk pointer to chunk
   *
   * @return true if chunk processed correctly
   *
   * @note this is one of the mechanisms to handle of extra chunk types
   * @note but see PostReadChunks below as well
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool ProcessChunk(RIFFChunk *chunk) {UNUSED_PARAMETER(chunk); return true;}

  /*--------------------------------------------------------------------------------*/
  /** Post chunk reading processing (called after all chunks read)
   *
   * @return true if processing successful
   *
   * @note this is the other way of handling extra chunk types, especially if there
   * @note are dependencies between chunk types
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool PostReadChunks() {return true;}

  /*--------------------------------------------------------------------------------*/
  /** Overrideable called whenever sample position changes
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdateSamplePosition() {}

protected:
  SoundFile         *file;
  uint8_t           filetype;
  const SoundFormat *fileformat;
  SoundFileSamples  *filesamples;

  std::vector<RIFFChunk *>        chunklist;
  std::map<uint32_t, RIFFChunk *> chunkmap;
};

BBC_AUDIOTOOLBOX_END

#endif
