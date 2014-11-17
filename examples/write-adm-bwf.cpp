
#include <stdio.h>
#include <stdlib.h>

#include <bbcat-audioobjects/ADMRIFFFile.h>
#include <bbcat-audioobjects/TinyXMLADMData.h>

using namespace bbcat;

int main(void)
{
  // register the TinyXML implementation of ADMData handler as usable
  TinyXMLADMData::Register();

  // ADM aware WAV file
  ADMRIFFFile file;
  const char *filename = "adm-bwf.wav";

  if (file.Create(filename, 48000, 16))
  {
    // create basic ADM
    if (file.CreateADM())
    {
      ADMData::OBJECTNAMES names;
      ADMData *adm = file.GetADM();

      printf("Created '%s' okay, %u channels at %luHz (%u bytes per sample)\n", filename, file.GetChannels(), (ulong_t)file.GetSampleRate(), (uint_t)file.GetBytesPerSample());

      // set programme name
      // if an audioProgramme object of this name doesn't exist, one will be created
      names.programmeName = "ADM Test Programme";

      // set content name
      // if an audioContent object of this name doesn't exist, one will be created
      names.contentName   = "ADM Test Content";
    
      // create tracks, channels and streams
      uint_t t;
      const ADMData::TRACKLIST& tracklist = adm->GetTrackList();
      for (t = 0; t < tracklist.size(); t++)
      {
        std::string trackname;

        DEBUG("------------- Track %2u -----------------", t + 1);

        // create default audioTrackFormat name (used for audioStreamFormat objects as well)
        Printf(trackname, "Track %u", t + 1);

        names.trackNumber = t;

        // derive channel and stream names from track name
        names.channelFormatName = trackname;
        names.streamFormatName  = "PCM_" + trackname;
        names.trackFormatName   = "PCM_" + trackname;

        // set object name
        // create 4 objects, each of 4 tracks
        names.objectName = "";  // need this because Printf() APPENDS!
        Printf(names.objectName, "Object %u", 1 + (t / 4));
        
        // set pack name from object name
        // create 4 packs, each of 4 tracks
        names.packFormatName = "";  // need this because Printf() APPENDS!
        Printf(names.packFormatName, "Pack %u", 1 + (t / 4));

        adm->CreateObjects(names);

        // note how the programme and content names are left in place in 'names'
        // this is necessary to ensure that things are linked up properly
      }
    }
    else fprintf(stderr, "Failed to create ADM for file '%s'!\n", filename);
    
    // write audio
    std::vector<float> audio;
    audio.resize(file.GetChannels());
    double fs = (double)file.GetSampleRate();
    double level = pow(10.0, .05 * -40.0);
    uint_t i, j, nsamples = 20 * file.GetSampleRate(), step = file.GetSampleRate() / 10;

    // write 20s worth samples
    // this is somewhat inefficient since it is only writing one frame at a time!
    for (i = 0; i < nsamples; i++)
    {
      // every 1/10s set positions of channels
      if (!(i % step))
      {
        uint_t index = i / step;

        for (j = 0; j < audio.size(); j++)
        {
          Position pos;

          pos.polar = true;
          pos.pos.az = (double)(index + j) * 20.0;
          pos.pos.el = (double)j / (double)file.GetChannels() * 60.0;
          pos.pos.d  = 1.0;
          file.SetPosition(j, pos);
        }
      }

      // create sine waves on each channel
      for (j = 0; j < audio.size(); j++)
      {
        audio[j] = level * sin(2.0 * M_PI * 100.0 * (double)i / fs * (double)(1 + j));
      }

      // write a frame of audio
      file.WriteSamples(&audio[0], 0, audio.size(), 1);
    }

    file.Close();
  }
  else fprintf(stderr, "Failed to open file '%s' for writing!\n", filename);

  return 0;
}

