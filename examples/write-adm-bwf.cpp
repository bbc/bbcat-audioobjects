
#include <stdio.h>
#include <stdlib.h>

#include <bbcat-audioobjects/ADMRIFFFile.h>
#include <bbcat-audioobjects/ADMAudioFileSamples.h>
#include <bbcat-audioobjects/TinyXMLADMData.h>

USE_BBC_AUDIOTOOLBOX;

int main(void)
{
  // register TineXML based ADM decoder
  TinyXMLADMData::Register();

  // ADM aware WAV file
  ADMRIFFFile file;
  const char *filename = "adm-bwf.wav";

  if (file.Create(filename, 48000, 16))
  {
    // create basic ADM
    if (file.CreateADM())
    {
      ADMData           *adm = file.GetADM();
      ADMAudioProgramme *programme = NULL;
      ADMAudioContent   *content   = NULL;
      ADMAudioObject    *object1, *object2;

      printf("Created '%s' okay, %u channels at %luHz (%u bytes per sample)\n", filename, file.GetChannels(), (ulong_t)file.GetSampleRate(), (uint_t)file.GetBytesPerSample());

      // create programme
      programme = adm->CreateProgramme("ADM Test Programme");
    
      // create content and link it to programme
      content   = adm->CreateContent("ADM Test Content", programme);

      // create a couple of objects and link them to the content
      object1   = adm->CreateObject("Object 1", content);
      object2   = adm->CreateObject("Object 2", content);

      // create tracks, channels and streams
      uint_t t;
      const ADMData::TRACKLIST& tracklist = adm->GetTrackList();
      for (t = 0; t < tracklist.size(); t++)
      {
        std::string trackname, objectname;

        // create default audioTrackFormat name (used for audioStreamFormat objects as well)
        Printf(trackname, "Track %u", t + 1);

        // pick object 1 or 2 for this track
        ADMAudioObject *object = (t < (tracklist.size() / 2)) ? object1 : object2;

        // get object name
        objectname = object->GetName();
      
        // use object name as pack name and see if it already exists
        ADMAudioPackFormat    *packFormat    = dynamic_cast<ADMAudioPackFormat *>(const_cast<ADMObject *>(adm->GetObjectByName(objectname, ADMAudioPackFormat::Type)));
        ADMAudioChannelFormat *channelFormat = adm->CreateChannelFormat(trackname);
        ADMAudioStreamFormat  *streamFormat  = adm->CreateStreamFormat("PCM_" + trackname);
        ADMAudioTrackFormat   *trackFormat   = adm->CreateTrackFormat("PCM_" + trackname);
        ADMAudioTrack         *audioTrack    = const_cast<ADMAudioTrack *>(tracklist[t]);

        // the same pack can cover a number of tracks
        if (packFormat == NULL)
        {
          packFormat = adm->CreatePackFormat(objectname);
          // set pack type
          packFormat->SetTypeLabel("0003");
          packFormat->SetTypeDefinition("Objects");
        }

        // set channel type
        channelFormat->SetTypeLabel("0003");
        channelFormat->SetTypeDefinition("Objects");

        // set track type (PCM)
        trackFormat->SetFormatLabel("0001");
        trackFormat->SetFormatDefinition("PCM");

        // set stream type (PCM)
        streamFormat->SetFormatLabel("0001");
        streamFormat->SetFormatDefinition("PCM");

        // connect ADM objects
        packFormat->Add(channelFormat);
        trackFormat->Add(streamFormat);
        streamFormat->Add(trackFormat);
        streamFormat->Add(channelFormat);
        audioTrack->Add(trackFormat);
        audioTrack->Add(packFormat);

        // add the pack to the object
        object->Add(packFormat);
        // add the track to the object
        object->Add(audioTrack);
      }
    }
    else fprintf(stderr, "Failed to create ADM for file '%s'!\n", filename);
    
    // write audio

    file.Close();
  }
  else fprintf(stderr, "Failed to open file '%s' for writing!\n", filename);

  return 0;
}

