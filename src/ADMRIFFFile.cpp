
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <string>

#define DEBUG_LEVEL 2

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
/** Create a WAVE/RIFF file
 *
 * @param filename filename of file to create
 * @param samplerate sample rate of audio
 * @param nchannels number of audio channels
 * @param format sample format of audio in file
 *
 * @return true if file created properly
 */
/*--------------------------------------------------------------------------------*/
bool ADMRIFFFile::Create(const char *filename, uint32_t samplerate, uint_t nchannels, SampleFormat_t format)
{
  bool success = false;

  if ((adm = ADMData::Create()) != NULL)
  {
    if (RIFFFile::Create(filename, samplerate, nchannels, format))
    {
      uint_t i;

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

      success = true;
    }
  }
  else ERROR("No providers for ADM XML decoding!");

  return success;
}

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

    // set up chna and axml chunks
    if ((chna = adm->GetChna(chnalen)) != NULL)
    {
      if ((chunk = AddChunk(chna_ID)) != NULL)
      {
        chunk->CreateWriteData(chna, chnalen);
      }
      else ERROR("Failed to add chna chunk");

      delete[] chna;
    }
    else ERROR("No chna data available");

    if ((chunk = AddChunk(axml_ID)) != NULL)
    {
      std::string str = adm->GetAxml();

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

  if (success)
  {
    RIFFChunk *chna = GetChunk(chna_ID);
    RIFFChunk *axml = GetChunk(axml_ID);

    if (adm &&
        chna && chna->GetData() &&
        axml && axml->GetData())
    {
      success = adm->Set(chna->GetData(), axml->GetData(), axml->GetLength());

#if DEBUG_LEVEL >= 4
      {
        std::string str;
        adm->Dump(str);
                
        DEBUG("%s", str.c_str());
      }

      {
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
    else
    {
      if (!adm)                       ERROR("Cannot decode ADM, no ADM decoder available");
      if (!(chna && chna->GetData())) ERROR("Cannot decode ADM, chna chunk not available");
      if (!(axml && axml->GetData())) ERROR("Cannot decode ADM, chna chunk not available");
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
