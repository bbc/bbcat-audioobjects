
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_LEVEL 1
#include "SoundFileAttributes.h"

BBC_AUDIOTOOLBOX_START

SoundFile::SoundFile() : fp(NULL)
{
}

SoundFile::SoundFile(const SoundFile& obj) : fp(NULL)
{
  operator = (obj);
}

SoundFile::~SoundFile()
{
  fclose();
}

SoundFile& SoundFile::operator = (const SoundFile& obj)
{
  fclose();

  if (obj.isopen())
  {
    if (fopen(obj.filename.c_str(), obj.mode.c_str()))
    {
      fseek(obj.ftell(), SEEK_SET);
    }
  }

  return *this;
}

bool SoundFile::fopen(const char *filename, const char *mode)
{
  bool success = false;

  if (!isopen())
  {
    if ((fp = ::fopen(filename, mode)) != NULL)
    {
      DEBUG2(("Opened '%s' for '%s'", filename, mode));
      this->filename = filename;
      this->mode     = mode;
      success        = true;
    }
    else DEBUG2(("Failed to open '%s' for '%s'", filename, mode));
  }

  return success;
}

void SoundFile::fclose()
{
  if (fp)
  {
    ::fclose(fp);
    fp = NULL;

    filename = "";
    mode     = "";
  }
}

/*----------------------------------------------------------------------------------------------------*/

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

SoundFileSamples::SoundFileSamples() : format(NULL),
                                       file(NULL),
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

SoundFileSamples::SoundFileSamples(const SoundFileSamples *obj) : format(NULL),
                                                                  file(NULL),
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
  SetFile(obj->file, obj->filepos, obj->totalbytes);
  SetClip(obj->GetClip());
}

SoundFileSamples::~SoundFileSamples()
{
  if (samplebuffer) delete[] samplebuffer;
  if (file)         delete file;
}

void SoundFileSamples::SetFormat(const SoundFormat *format)
{
  this->format = format;
  UpdateData();
}

void SoundFileSamples::SetFile(const SoundFile *file, ulong_t pos, ulong_t bytes, bool readonly)
{
  if (this->file) delete this->file;

  this->file = file ? file->dup() : NULL;

  filepos    = pos;
  totalbytes = bytes;

  this->readonly = readonly;

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

void SoundFileSamples::UpdateData()
{
  if (format)
  {
    totalsamples = totalbytes / format->GetBytesPerFrame();
    if (samplebuffer) delete[] samplebuffer;
    samplebuffer = new uint8_t[samplebufferframes * format->GetChannels() * sizeof(double)];

    Clip_t newclip =
    {
      0, ~(ulong_t)0,
      0, ~(uint_t)0,
    };
    SetClip(newclip);
  }
}

BBC_AUDIOTOOLBOX_END
