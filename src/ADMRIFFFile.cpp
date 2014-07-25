
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
                             adm(ADMData::Create())
{
  if (!adm) {
    ERROR("No providers for ADM XML decoding!");
  }
}

ADMRIFFFile::~ADMRIFFFile()
{
  Close();
}

void ADMRIFFFile::Close()
{
  RIFFFile::Close();

  if (adm) {
    adm->Delete();
  }
}

bool ADMRIFFFile::PostReadChunks()
{
  bool success = RIFFFile::PostReadChunks();

  if (success) {
    RIFFchnaChunk *chna = dynamic_cast<RIFFchnaChunk *>(GetChunk("chna"));
    RIFFaxmlChunk *axml = dynamic_cast<RIFFaxmlChunk *>(GetChunk("axml"));

    if (adm &&
        chna && chna->GetData() &&
        axml && axml->GetData()) {
      success = adm->Set(chna->GetData(), axml->GetData(), axml->GetLength());

#if 0
      {
        string str;
        adm->Dump(str);
                
        DEBUG("%s", str.c_str());
      }

      {
        string str;
        adm->GenerateXML(str);
                
        DEBUG("%s", str.c_str());
      }

      DEBUG("Audio objects:");
      vector<const ADMObject *>list;
      adm->GetADMList(ADMAudioObject::Type, list);
      uint_t i;
      for (i = 0; i < list.size(); i++) {
        DEBUG("%s", list[i]->ToString().c_str());
      }
#endif
    }
    else {
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
