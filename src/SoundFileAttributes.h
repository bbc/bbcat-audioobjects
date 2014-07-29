#ifndef __SOUND_FILE_ATTRIBUTES__
#define __SOUND_FILE_ATTRIBUTES__

#include <string>

#include <aplibs-dsp/misc.h>

#include <aplibs-dsp/SoundFormatConversions.h>

BBC_AUDIOTOOLBOX_START

class SoundFile
{
public:
  SoundFile();
  SoundFile(const SoundFile& obj);
  virtual ~SoundFile();

  SoundFile& operator = (const SoundFile& obj);

  virtual SoundFile *dup() const {return new SoundFile(*this);}

  virtual bool     fopen(const char *filename, const char *mode = "rb");
  bool             isopen() const {return (fp != NULL);}
  virtual void     fclose();

  operator FILE *() {return fp;}

  virtual size_t   fread(void *ptr, size_t size, size_t count)  {return ::fread(ptr, size, count, fp);}
  virtual size_t   fwrite(void *ptr, size_t size, size_t count) {return ::fwrite(ptr, size, count, fp);}
  virtual long int ftell() const {return ::ftell(fp);}
  virtual int      fseek(long int offset, int origin) {return ::fseek(fp, offset, origin);}
  virtual int      ferror() const {return ::ferror(fp);}

protected:
  std::string filename;
  std::string mode;
  FILE        *fp;
};

class SoundFormat
{
public:
  SoundFormat();
  virtual ~SoundFormat();

  virtual uint32_t       GetSampleRate()       const {return samplerate;}
  virtual uint_t         GetChannels()         const {return channels;}
  virtual uint8_t        GetBytesPerSample()   const {return bytespersample;}
  virtual uint_t         GetBytesPerFrame()    const {return channels * bytespersample;}
  virtual SampleFormat_t GetSampleFormat()     const {return format;}
  virtual bool           GetSamplesBigEndian() const {return bigendian;}

protected:
  uint32_t       samplerate;
  uint_t         channels;
  uint8_t        bytespersample;
  SampleFormat_t format;
  bool           bigendian;
};

class SoundFileSamples
{
public:
  SoundFileSamples();
  SoundFileSamples(const SoundFileSamples *obj);
  virtual ~SoundFileSamples();

  virtual SoundFileSamples *Duplicate() const {return new SoundFileSamples(this);}

  virtual void SetSampleBufferSize(uint_t samples = 256) {samplebufferframes = samples; UpdateData();}

  virtual void SetFormat(const SoundFormat *format);
  const SoundFormat *GetFormat() const {return format;}
  virtual void SetFile(const SoundFile *file, ulong_t pos, ulong_t bytes, bool readonly = true);

  uint_t  GetStartChannel() const {return clip.channel;}
  uint_t  GetChannels()     const {return clip.nchannels;}

  ulong_t GetPosition()         const {return samplepos;}
  ulong_t GetLength()           const {return clip.nsamples;}
  ulong_t GetAbsolutePosition() const {return clip.start + samplepos;}
  ulong_t GetAbsoluteLength()   const {return clip.start + clip.nsamples;}

  void    SetPosition(ulong_t pos)         {samplepos = MIN(pos, clip.nsamples); UpdatePosition();}
  void    SetAbsolutePosition(ulong_t pos) {samplepos = LIMIT(pos, clip.start, clip.start + clip.nsamples) - clip.start; UpdatePosition();}

  typedef struct
  {
    ulong_t start;
    ulong_t nsamples;
    uint_t  channel;
    uint_t  nchannels;
  } Clip_t;
  const Clip_t& GetClip() const {return clip;}
  void SetClip(const Clip_t& newclip);

  virtual uint_t ReadSamples(uint8_t  *buffer, SampleFormat_t type, uint_t frames, uint_t firstchannel = 0, uint_t nchannels = ~0);
  virtual uint_t ReadSamples(sint16_t *dst, uint_t frames, uint_t firstchannel = 0, uint_t nchannels = ~0) {return ReadSamples((uint8_t *)dst, SampleFormatOf(dst), frames, firstchannel, nchannels);}
  virtual uint_t ReadSamples(sint32_t *dst, uint_t frames, uint_t firstchannel = 0, uint_t nchannels = ~0) {return ReadSamples((uint8_t *)dst, SampleFormatOf(dst), frames, firstchannel, nchannels);}
  virtual uint_t ReadSamples(float    *dst, uint_t frames, uint_t firstchannel = 0, uint_t nchannels = ~0) {return ReadSamples((uint8_t *)dst, SampleFormatOf(dst), frames, firstchannel, nchannels);}
  virtual uint_t ReadSamples(double   *dst, uint_t frames, uint_t firstchannel = 0, uint_t nchannels = ~0) {return ReadSamples((uint8_t *)dst, SampleFormatOf(dst), frames, firstchannel, nchannels);}

protected:
  virtual void UpdateData();
  virtual void UpdatePosition() {}

protected:
  const SoundFormat *format;
  SoundFile             *file;
  Clip_t                clip;
  ulong_t               filepos;
  ulong_t               samplepos;
  ulong_t               totalsamples;
  ulong_t               totalbytes;
  uint8_t               *samplebuffer;
  uint_t                samplebufferframes;
  bool                  readonly;
};

BBC_AUDIOTOOLBOX_END

#endif
