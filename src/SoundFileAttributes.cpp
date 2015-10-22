
#include <string.h>

#define DEBUG_LEVEL 1
#include "SoundFileAttributes.h"

BBC_AUDIOTOOLBOX_START

SoundFormat::SoundFormat() : samplerate(0),
                             channels(0),
                             bytespersample(0),
                             format(SampleFormat_Unknown),
                             bigendian(false)
{
}

SoundFormat::~SoundFormat()
{
}

/*----------------------------------------------------------------------------------------------------*/

SoundFileSamples::SoundFileSamples() :
  format(NULL),
  filepos(0),
  samplepos(0),
  totalsamples(0),
  totalbytes(0),
  samplebuffer(NULL),
  samplebufferframes(256),
  readonly(true)
{
  memset(&clip, 0, sizeof(clip));
}

SoundFileSamples::SoundFileSamples(const SoundFileSamples *obj) :
  format(NULL),
  filepos(0),
  samplepos(0),
  totalsamples(0),
  totalbytes(0),
  samplebuffer(NULL),
  samplebufferframes(256),
  readonly(true)
{
  memset(&clip, 0, sizeof(clip));

  SetFormat(obj->GetFormat());
  SetFile(obj->fileref, obj->filepos, obj->totalbytes);
  SetClip(obj->GetClip());
}

SoundFileSamples::~SoundFileSamples()
{
  if (samplebuffer) delete[] samplebuffer;

  EnhancedFile *file;
  if ((file = fileref.Obj()) != NULL)
  {
    std::string filename = file->getfilename();

    file->fclose();

    // DON'T delete file object here, it will be done by the fileref object on destruction
  }
}

void SoundFileSamples::SetFormat(const SoundFormat *format)
{
  this->format = format;
  UpdateData();
}

void SoundFileSamples::SetFile(const RefCount<EnhancedFile>& file, uint64_t pos, uint64_t bytes, bool readonly)
{
  // use file reference to control deletion
  fileref    = file;

  filepos    = pos;
  totalbytes = bytes;

  this->readonly = readonly;

  UpdateData();
}

void SoundFileSamples::SetClip(const Clip_t& newclip)
{
  clip = newclip;
  clip.start     = MIN(clip.start,     totalsamples);
  clip.nsamples  = MIN(clip.nsamples,  totalsamples - clip.start);
  clip.channel   = MIN(clip.channel,   format->GetChannels());
  clip.nchannels = MIN(clip.nchannels, format->GetChannels() - clip.channel);

  samplepos = MIN(samplepos, clip.nsamples);
  UpdatePosition();
}

uint_t SoundFileSamples::ReadSamples(uint8_t *buffer, SampleFormat_t type, uint_t dstchannel, uint_t ndstchannels, uint_t frames, uint_t firstchannel, uint_t nchannels)
{
  EnhancedFile *file = fileref;
  uint_t n = 0;

  if (file && file->isopen() && samplebuffer)
  {
    frames = (uint_t)MIN((uint64_t)frames, clip.nsamples - samplepos);

    if (!frames)
    {
      DEBUG3(("No sample data left (pos = %lu, nsamples = %lu)!", (ulong_t)samplepos, (ulong_t)clip.nsamples));
    }

    firstchannel = MIN(firstchannel, clip.nchannels);
    nchannels    = MIN(nchannels,    clip.nchannels - firstchannel);

    dstchannel   = MIN(dstchannel,   ndstchannels);
    nchannels    = MIN(nchannels,    ndstchannels - dstchannel);

    n = 0;
    if (nchannels)
    {
      while (frames)
      {
        uint_t nframes = MIN(frames, samplebufferframes);
        size_t res;

        DEBUG4(("Seeking to %lu", (ulong_t)(filepos + (clip.start + samplepos) * format->GetBytesPerFrame())));
        if (file->fseek(filepos + samplepos * format->GetBytesPerFrame(), SEEK_SET) == 0)
        {
          DEBUG4(("Reading %u x %u bytes", nframes, format->GetBytesPerFrame()));

          if ((res = file->fread(samplebuffer, format->GetBytesPerFrame(), nframes)) > 0)
          {
            nframes = (uint_t)res;

            DEBUG4(("Read %u frames, extracting channels %u-%u (from 0-%u), converting and copying to destination", nframes, clip.channel + firstchannel, clip.channel + firstchannel + nchannels, format->GetChannels()));

            // de-interleave, convert and transfer samples
            TransferSamples(samplebuffer, format->GetSampleFormat(), format->GetSamplesBigEndian(), clip.channel + firstchannel, format->GetChannels(),
                            buffer, type, MACHINE_IS_BIG_ENDIAN, dstchannel, ndstchannels,
                            nchannels,
                            nframes);

            n         += nframes;
            buffer    += nframes * ndstchannels * GetBytesPerSample(type);
            frames    -= nframes;
            samplepos += nframes;
          }
          else if (res <= 0)
          {
            ERROR("Failed to read %u frames (%u bytes) from file, error %s", nframes, nframes * format->GetBytesPerFrame(), strerror(file->ferror()));
            break;
          }
          else
          {
            DEBUG3(("No data left!"));
            break;
          }
        }
        else
        {
          ERROR("Failed to seek to correct position in file, error %s", strerror(file->ferror()));
          n = 0;
          break;
        }
      }
    }
    else
    {
      // no channels to transfer, just increment position and return number of requested frames
      n          = frames;
      samplepos += n;
    }

    UpdatePosition();
  }
  else ERROR("No file or sample buffer");

  return n;
}

uint_t SoundFileSamples::WriteSamples(const uint8_t *buffer, SampleFormat_t type, uint_t srcchannel, uint_t nsrcchannels, uint_t nsrcframes, uint_t firstchannel, uint_t nchannels)
{
  EnhancedFile *file = fileref;
  uint_t n = 0;

  if (file && file->isopen() && samplebuffer && !readonly)
  {
    uint_t bpf = format->GetBytesPerFrame();

    firstchannel = MIN(firstchannel, clip.nchannels);
    nchannels    = MIN(nchannels,    clip.nchannels - firstchannel);

    srcchannel   = MIN(srcchannel,   nsrcchannels);
    nchannels    = MIN(nchannels,    nsrcchannels - srcchannel);

    n = 0;
    if (nchannels)
    {
      while (nsrcframes)
      {
        uint_t nframes = MIN(nsrcframes, samplebufferframes);
        size_t res;

        if (nchannels < format->GetChannels())
        {
          // read existing sample data to allow overwriting of channels
          res = file->fread(samplebuffer, bpf, nframes);

          // clear rest of buffer
          if (res < (bpf * nframes)) memset(samplebuffer + res * bpf, 0, (nframes - res) * bpf);

          // move back in file for write
          if (res) file->fseek(-(long)(res * bpf), SEEK_CUR);
        }

        // copy/interleave/convert samples
        TransferSamples(buffer, type, MACHINE_IS_BIG_ENDIAN, srcchannel, nsrcchannels,
                        samplebuffer, format->GetSampleFormat(), format->GetSamplesBigEndian(), clip.channel + firstchannel, nchannels,
                        ~0,       // number of channels actually transfer will be limited by nsrcchannels and nchannels above
                        nframes);

        if ((res = file->fwrite(samplebuffer, bpf, nframes)) > 0)
        {
          nframes     = (uint_t)res;
          n          += nframes;
          buffer     += nframes * nsrcchannels * GetBytesPerSample(type);
          nsrcframes -= nframes;
          samplepos  += nframes;

          totalsamples  = MAX(totalsamples,  samplepos);
          clip.nsamples = MAX(clip.nsamples, totalsamples - clip.start);

          totalbytes    = totalsamples * format->GetBytesPerFrame();
        }
        else if (res <= 0)
        {
          ERROR("Failed to write %u frames (%u bytes) to file, error %s", nframes, nframes * bpf, strerror(file->ferror()));
          break;
        }
        else
        {
          DEBUG3(("No data left!"));
          break;
        }
      }
    }
    else
    {
      // no channels to transfer, just increment position and return number of requested frames
      n          = nsrcframes;
      samplepos += n;
    }

    UpdatePosition();
  }
  else ERROR("No file or sample buffer");

  return n;
}

void SoundFileSamples::UpdateData()
{
  if (format)
  {
    totalsamples = totalbytes / format->GetBytesPerFrame();
    if (samplebuffer) delete[] samplebuffer;
    samplebuffer = new uint8_t[samplebufferframes * format->GetChannels() * sizeof(double)];

    Clip_t newclip =
    {
      0, ~(uint64_t)0,
      0, ~(uint_t)0,
    };
    SetClip(newclip);

    timebase = format->GetTimeBase();
  }
}

BBC_AUDIOTOOLBOX_END
