
#include <stdio.h>
#include <stdlib.h>
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
  file(NULL),
  filepos(0),
  samplepos(0),
  totalsamples(0),
  totalbytes(0),
  samplebuffer(NULL),
  samplebufferframes(256),
  readonly(true),
  istempfile(false)
{
  memset(&clip, 0, sizeof(clip));
}

SoundFileSamples::SoundFileSamples(const SoundFileSamples *obj) :
  format(NULL),
  file(NULL),
  filepos(0),
  samplepos(0),
  totalsamples(0),
  totalbytes(0),
  samplebuffer(NULL),
  samplebufferframes(256),
  readonly(true),
  istempfile(false)
{
  memset(&clip, 0, sizeof(clip));

  SetFormat(obj->GetFormat());
  SetFile(obj->file, obj->filepos, obj->totalbytes);
  SetClip(obj->GetClip());
}

SoundFileSamples::~SoundFileSamples()
{
  if (samplebuffer) delete[] samplebuffer;
  if (file)
  {
    std::string filename = file->getfilename();

    file->fclose();

    // delete file if it has been used as a temporary file
    if (istempfile) remove(filename.c_str());

    delete file;
  }
}

bool SoundFileSamples::CreateTempFile()
{
  std::string filename;
  bool success = false;

  if (!file) file = new EnhancedFile;

  if (file)
  {
    // create temporary file for samples
    Printf(filename, "samples-%016lx.raw", (ulong_t)this);

    success    = file->fopen(filename.c_str(), "wb+");
    readonly   = false;
    istempfile = true;
  }

  return success;
}

void SoundFileSamples::SetFormat(const SoundFormat *format)
{
  this->format = format;
  UpdateData();
}

void SoundFileSamples::SetFile(const EnhancedFile *file, uint64_t pos, uint64_t bytes, bool readonly)
{
  if (this->file) delete this->file;

  this->file = file ? file->dup() : NULL;

  filepos    = pos;
  totalbytes = bytes;

  this->readonly = readonly;

  UpdateData();
}

void SoundFileSamples::Set64bitLength(uint64_t bytes)
{
  totalbytes = bytes;

  UpdateData();
}

void SoundFileSamples::SetClip(const Clip_t& newclip)
{
  clip = newclip;
  clip.start     = MIN(clip.start,     totalsamples - 1);
  clip.nsamples  = MIN(clip.nsamples,  totalsamples - clip.start);
  clip.channel   = MIN(clip.channel,   format->GetChannels() - 1);
  clip.nchannels = MIN(clip.nchannels, format->GetChannels() - clip.channel);

  samplepos = MIN(samplepos, clip.nsamples);
  UpdatePosition();
}

uint_t SoundFileSamples::ReadSamples(uint8_t *buffer, SampleFormat_t type, uint_t frames, uint_t firstchannel, uint_t nchannels)
{
  uint_t n = 0;

  if (file && file->isopen() && samplebuffer)
  {
    frames = MIN(frames, clip.nsamples - samplepos);

    if (!frames)
    {
      DEBUG3(("No sample data left (pos = %lu, nsamples = %lu)!", (ulong_t)samplepos, (ulong_t)clip.nsamples));
    }

    firstchannel = MIN(firstchannel, clip.nchannels - 1);
    nchannels    = MIN(nchannels,    clip.nchannels - firstchannel);

    n = 0;
    while (frames)
    {
      uint_t nframes = MIN(frames, samplebufferframes);
      size_t res;

      DEBUG4(("Seeking to %lu", filepos + (clip.start + samplepos) * format->GetBytesPerFrame()));
      if (file->fseek(filepos + samplepos * format->GetBytesPerFrame(), SEEK_SET) == 0)
      {
        DEBUG4(("Reading %u x %u bytes", nframes, format->GetBytesPerFrame()));

        if ((res = file->fread(samplebuffer, format->GetBytesPerFrame(), nframes)) > 0)
        {
          nframes = res;

          DEBUG4(("Read %u frames, extracting channels %u-%u (from 0-%u), converting and copying to destination", nframes, clip.channel + firstchannel, clip.channel + firstchannel + nchannels, format->GetChannels()));

          // de-interleave, convert and transfer samples
          TransferSamples(samplebuffer, format->GetSampleFormat(), format->GetSamplesBigEndian(), clip.channel + firstchannel, format->GetChannels(),
                          buffer, type, MACHINE_IS_BIG_ENDIAN, 0, nchannels,
                          nchannels,
                          nframes);

          n         += nframes;
          buffer    += nframes * nchannels * GetBytesPerSample(type);
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

    UpdatePosition();
  }
  else ERROR("No file or sample buffer");

  return n;
}

uint_t SoundFileSamples::WriteSamples(const uint8_t *buffer, SampleFormat_t type, uint_t srcchannel, uint_t nsrcchannels, uint_t nsrcframes, uint_t firstchannel, uint_t nchannels)
{
  uint_t n = 0;

  if (file && file->isopen() && samplebuffer && !readonly)
  {
    uint_t bpf = format->GetBytesPerFrame();

    firstchannel = MIN(firstchannel, clip.nchannels - 1);
    nchannels    = MIN(nchannels,    clip.nchannels - firstchannel);

    n = 0;
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
        nframes     = res;
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
