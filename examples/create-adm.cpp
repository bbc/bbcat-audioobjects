
#include <stdio.h>
#include <stdlib.h>

#include <bbcat-audioobjects/TinyXMLADMData.h>

using namespace bbcat;

int main(void)
{
  // register the TinyXML implementation of ADMData handler as usable
  TinyXMLADMData::Register();

  ADMData *adm;
  // create basic ADM
  if ((adm = ADMData::Create()) != NULL)
  {
    ADMData::OBJECTNAMES names;

    // set programme name
    // if an audioProgramme object of this name doesn't exist, one will be created
    names.programmeName = "ADM Test Programme";

    // set content name
    // if an audioContent object of this name doesn't exist, one will be created
    names.contentName   = "ADM Test Content";
    
    // create 16 tracks, channels and streams
    uint_t t, ntracks = 16;
    for (t = 0; t < ntracks; t++)
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

      // find channel format object for this track
      ADMAudioChannelFormat *cf;
      if ((cf = dynamic_cast<ADMAudioChannelFormat *>(adm->GetWritableObjectByName(names.channelFormatName, ADMAudioChannelFormat::Type))) != NULL)
      {
        // found channel format, generate block formats for it
        ADMAudioBlockFormat   *bf;
        uint_t i;

        for (i = 0; i < 20; i++)
        {
          if ((bf = adm->CreateBlockFormat("" /* block formats do not have names */, cf)) != NULL)
          {
            AudioObjectParameters params;
            Position pos;
              
            // set start time to be index * 25ms
            bf->SetRTime(i * 25000000);
            // set duration to be 25ms
            bf->SetDuration(25000000);

            pos.polar  = true;
            pos.pos.az = fmod((double)(t + i) * 20.0, 360.0);
            pos.pos.el = (double)i / (double)ntracks * 60.0;
            pos.pos.d  = 1.0;
            params.SetPosition(pos);
            
            // set object parameters
            bf->GetObjectParameters() = params;
          }
          else ERROR("Failed to create audioBlockFormat (t: %u, i: %u)", t, i);
        }
      }
      else
      {
        // this will only occur in the case of an error
        ERROR("Unable to find channel format '%s'", names.channelFormatName.c_str());
      }
    }

    // finalise ADM
    adm->SortTracks();
    adm->ConnectReferences();
    adm->ChangeTemporaryIDs();

    // output ADM
    printf("XML:\n%s", adm->GetAxml().c_str());
  }

  return 0;
}

