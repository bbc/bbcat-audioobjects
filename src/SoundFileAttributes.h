#ifndef __SOUND_FILE_ATTRIBUTES__
#define __SOUND_FILE_ATTRIBUTES__

#include <string>

#include <aplibs-dsp/misc.h>
#include <aplibs-dsp/EnhancedFile.h>

#include <aplibs-dsp/SoundFormatConversions.h>
#include <aplibs-dsp/UniversalTime.h>

BBC_AUDIOTOOLBOX_START

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

  virtual void           SetSampleRate(uint32_t val)         {samplerate = val; timebase.SetDenominator(samplerate); timebase.Reset();}
  virtual void           SetChannels(uint_t val)             {channels = val;}
  virtual void           SetSampleFormat(SampleFormat_t val) {format = val; bytespersample = bbcat::GetBytesPerSample(format);}
  virtual void           SetSamplesBigEndian(bool val)       {bigendian = val;}

  const UniversalTime&   GetTimeBase() const {return timebase;}

protected:
  uint32_t       samplerate;
  uint_t         channels;
  uint8_t        bytespersample;
  SampleFormat_t format;
  bool           bigendian;
  UniversalTime  timebase;
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
  virtual void SetFile(const EnhancedFile *file, ulong_t pos, ulong_t bytes, bool readonly = true);

  uint_t  GetStartChannel()             const {return clip.channel;}
  uint_t  GetChannels()                 const {return clip.nchannels;}

  ulong_t GetSamplePosition()           const {return samplepos;}
  ulong_t GetSampleLength()             const {return clip.nsamples;}
  ulong_t GetAbsoluteSamplePosition()   const {return clip.start + samplepos;}
  ulong_t GetAbsoluteSampleLength()     const {return clip.start + clip.nsamples;}

  void    SetSamplePosition(ulong_t pos)         {samplepos = MIN(pos, clip.nsamples); UpdatePosition();}
  void    SetAbsoluteSamplePosition(ulong_t pos) {samplepos = LIMIT(pos, clip.start, clip.start + clip.nsamples) - clip.start; UpdatePosition();}

  uint64_t GetPositionNS()              const {return timebase.Calc(GetSamplePosition());}
  double   GetPositionSeconds()         const {return timebase.CalcSeconds(GetSamplePosition());}

  uint64_t GetAbsolutePositionNS()      const {return timebase.GetTime();}
  double   GetAbsolutePositionSeconds() const {return timebase.GetTimeSeconds();}

  const UniversalTime& GetTimeBase()    const {return timebase;}

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

  virtual uint_t WriteSamples(const uint8_t  *buffer, SampleFormat_t type, uint_t srcchannel, uint_t nsrcchannels, uint_t nsrcframes = 1, uint_t firstchannel = 0, uint_t nchannels = ~0);
  virtual uint_t WriteSamples(const sint16_t *dst, uint_t srcchannel, uint_t nsrcchannels, uint_t nsrcframes = 1, uint_t firstchannel = 0, uint_t nchannels = ~0) {return WriteSamples((const uint8_t *)dst, SampleFormatOf(dst), srcchannel, nsrcchannels, nsrcframes, firstchannel, nchannels);}
  virtual uint_t WriteSamples(const sint32_t *dst, uint_t srcchannel, uint_t nsrcchannels, uint_t nsrcframes = 1, uint_t firstchannel = 0, uint_t nchannels = ~0) {return WriteSamples((const uint8_t *)dst, SampleFormatOf(dst), srcchannel, nsrcchannels, nsrcframes, firstchannel, nchannels);}
  virtual uint_t WriteSamples(const float    *dst, uint_t srcchannel, uint_t nsrcchannels, uint_t nsrcframes = 1, uint_t firstchannel = 0, uint_t nchannels = ~0) {return WriteSamples((const uint8_t *)dst, SampleFormatOf(dst), srcchannel, nsrcchannels, nsrcframes, firstchannel, nchannels);}
  virtual uint_t WriteSamples(const double   *dst, uint_t srcchannel, uint_t nsrcchannels, uint_t nsrcframes = 1, uint_t firstchannel = 0, uint_t nchannels = ~0) {return WriteSamples((const uint8_t *)dst, SampleFormatOf(dst), srcchannel, nsrcchannels, nsrcframes, firstchannel, nchannels);}

protected:
  virtual void UpdateData();
  virtual void UpdatePosition() {timebase.Set(GetAbsoluteSamplePosition());}

  virtual bool CreateTempFile();

protected:
  const SoundFormat *format;
  UniversalTime     timebase;
  EnhancedFile      *file;
  Clip_t            clip;
  ulong_t           filepos;
  ulong_t           samplepos;
  ulong_t           totalsamples;
  ulong_t           totalbytes;
  uint8_t           *samplebuffer;
  uint_t            samplebufferframes;
  bool              readonly;
  bool              istempfile;
};

BBC_AUDIOTOOLBOX_END

#endif
