
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <string>

#define DEBUG_LEVEL 1

#include "ADMRIFFFile.h"
#include "RIFFChunk_Definitions.h"

BBC_AUDIOTOOLBOX_START

ADMRIFFFile::ADMRIFFFile() : RIFFFile(),
                             adm(NULL)
{
}

ADMRIFFFile::~ADMRIFFFile()
{
  Close();
}

/*--------------------------------------------------------------------------------*/
/** Open a WAVE/RIFF file
 *
 * @param filename filename of file to open
 *
 * @return true if file opened and interpreted correctly (including any extra chunks if present)
 */
/*--------------------------------------------------------------------------------*/
bool ADMRIFFFile::Open(const char *filename)
{
  bool success = false;

  if ((adm = ADMData::Create()) != NULL)
  {
    success = RIFFFile::Open(filename);
  }
  else ERROR("No providers for ADM XML decoding!");

  return success;
}


/*--------------------------------------------------------------------------------*/
/** Create ADM from text file
 *
 * @param filename text filename (see below for format)
 *
 * @return true if successful
 *
 * The file MUST be of the following format with each entry on its own line:
 * <ADM programme name>[:<ADM content name>]
 *
 * then for each track:
 * <track>:<trackname>:<objectname>
 *
 * Where <track> is 1..number of tracks available within ADM
 */
/*--------------------------------------------------------------------------------*/
bool ADMRIFFFile::CreateADM(const char *filename)
{
  bool success = false;

  if (IsOpen() && !adm)
  {
    if ((adm = ADMData::Create()) != NULL)
    {
      uint_t i, nchannels = GetChannels();

      for (i = 0; i < nchannels; i++)
      {
        ADMAudioTrack *track;
        std::string name;

        Printf(name, "Track %u", i + 1);

        if ((track = adm->CreateTrack(name)) != NULL)
        {
          track->SetTrackNum(i + 1);
          track->SetSampleRate(GetSampleRate());
          track->SetBitDepth(GetBitsPerSample());
        }
      }

      success = adm->CreateFromFile(filename);
    }
    else ERROR("No providers for ADM XML decoding!");
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Close RIFF file, writing chunks if file was opened for writing 
 */
/*--------------------------------------------------------------------------------*/
void ADMRIFFFile::Close()
{
  if (adm && writing)
  {
    RIFFChunk *chunk;
    uint32_t  chnalen;
    uint8_t   *chna;

    adm->SortTracks();
    adm->ConnectReferences();
    adm->UpdateLimits();

    // get ADM object to create chna chunk
    if ((chna = adm->GetChna(chnalen)) != NULL)
    {
      // and add it to the RIFF file
      if ((chunk = AddChunk(chna_ID)) != NULL)
      {
        chunk->CreateWriteData(chna, chnalen);
      }
      else ERROR("Failed to add chna chunk");

      // don't need the raw data any more
      delete[] chna;
    }
    else ERROR("No chna data available");

    // add axml chunk
    if ((chunk = AddChunk(axml_ID)) != NULL)
    {
      // create axml data
      std::string str = adm->GetAxml();

      // set chunk data
      chunk->CreateWriteData(str.c_str(), str.size());

      DEBUG3(("AXML: %s", str.c_str()));
    }
    else ERROR("Failed to add axml chunk");
  }

  // write chunks, copy samples and close file
  RIFFFile::Close();

  if (adm)
  {
    adm->Delete();
  }
}

bool ADMRIFFFile::PostReadChunks()
{
  bool success = RIFFFile::PostReadChunks();

  // after reading of chunks, find chna and axml chunks and decode them
  // to create an ADM
  if (success)
  {
    RIFFChunk *chna = GetChunk(chna_ID);
    RIFFChunk *axml = GetChunk(axml_ID);

    // ensure each chunk is valid
    if (adm &&
        chna && chna->GetData() &&
        axml && axml->GetData())
    {
      // decode chunks
      success = adm->Set(chna->GetData(), axml->GetData(), axml->GetLength());

#if DEBUG_LEVEL >= 4
      { // dump ADM as text
        std::string str;
        adm->Dump(str);
                
        DEBUG("%s", str.c_str());
      }

      { // dump ADM as XML
        std::string str;
        adm->GenerateXML(str);
                
        DEBUG("%s", str.c_str());
      }

      DEBUG("Audio objects:");
      std::vector<const ADMObject *> list;
      adm->GetADMList(ADMAudioObject::Type, list);
      uint_t i;
      for (i = 0; i < list.size(); i++)
      {
        DEBUG("%s", list[i]->ToString().c_str());
      }
#endif
    }
    // test for different types of failure
    else if (!adm)
    {
      ERROR("Cannot decode ADM, no ADM decoder available");
      success = false;
    }
    else if (!chna && !axml)
    {
      // acceptable failure - neither chna nor axml chunk specified - not an ADM compatible BWF file but open anyway
      DEBUG("Warning: no chna or axml chunks!");
      success = true;
    }
    else {
      // unacceptible failures: empty chna or empty axml chunks
      if (chna && !chna->GetData()) ERROR("Cannot decode ADM, chna chunk not available");
      if (axml && !axml->GetData()) ERROR("Cannot decode ADM, axml chunk not available");
      success = false;
    }

    // now that the data is dealt with, the chunk data can be deleted
    if (axml) axml->DeleteData();
    if (chna) chna->DeleteData();
  }

  return success;
}

void ADMRIFFFile::UpdateSamplePosition()
{
}

BBC_AUDIOTOOLBOX_END
