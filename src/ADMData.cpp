
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>

#define DEBUG_LEVEL 1
#include <bbcat-base/EnhancedFile.h>

#include "ADMData.h"
#include "RIFFChunk_Definitions.h"

BBC_AUDIOTOOLBOX_START

std::vector<ADMData::PROVIDER> ADMData::providerlist;

ADMData::ADMData()
{
}

ADMData::~ADMData()
{
  Delete();
}

/*--------------------------------------------------------------------------------*/
/** Delete all objects within this ADM
 */
/*--------------------------------------------------------------------------------*/
void ADMData::Delete()
{
  ADMOBJECTS_IT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    delete it->second;
  }

  admobjects.clear();
  tracklist.clear();
}

/*--------------------------------------------------------------------------------*/
/** Read ADM data from the chna RIFF chunk
 *
 * @param data ptr to chna chunk data 
 * @param len length of chna chunk
 *
 * @return true if data read successfully
 */
/*--------------------------------------------------------------------------------*/
bool ADMData::SetChna(const uint8_t *data, uint_t len)
{
  const CHNA_CHUNK& chna = *(const CHNA_CHUNK *)data;
  uint_t maxuids = (len - sizeof(CHNA_CHUNK)) / sizeof(chna.UIDs[0]);   // calculate maximum number of UIDs given chunk length
  std::string terminator;
  bool success = true;

  // create string with a single 0 byte in it to detect terminators
  terminator.push_back(0);

  if (maxuids < chna.UIDCount) ERROR("Warning: chna specifies %u UIDs but chunk has only length for %u", (uint_t)chna.UIDCount, maxuids);

  uint16_t i;
  for (i = 0; (i < chna.UIDCount) && (i < maxuids); i++)
  {
    // only handle non-zero track numbers
    if (chna.UIDs[i].TrackNum)
    {
      ADMAudioTrack *track;
      std::string id;

      id.assign(chna.UIDs[i].UID, sizeof(chna.UIDs[i].UID));

      if ((track = dynamic_cast<ADMAudioTrack *>(Create(ADMAudioTrack::Type, id, ""))) != NULL)
      {
        ADMVALUE value;

        value.attr = false;

        track->SetTrackNum(chna.UIDs[i].TrackNum - 1);

        value.name = ADMAudioTrackFormat::Reference;
        value.value.assign(chna.UIDs[i].TrackRef, sizeof(chna.UIDs[i].TrackRef));
        // trim any zero bytes off the end of the string
        value.value = value.value.substr(0, value.value.find(terminator));
        track->AddValue(value);
            
        value.name = ADMAudioPackFormat::Reference;
        value.value.assign(chna.UIDs[i].PackRef, sizeof(chna.UIDs[i].PackRef));
        // trim any zero bytes off the end of the string
        value.value = value.value.substr(0, value.value.find(terminator));
        track->AddValue(value);

        track->SetValues();
      }
      else ERROR("Failed to create AudioTrack for UID %u", i);
    }
  }

  SortTracks();
    
  return success;
}

/*--------------------------------------------------------------------------------*/
/** Read ADM data from the axml RIFF chunk
 *
 * @param data ptr to axml chunk data 
 * @param length length of axml data
 *
 * @return true if data read successfully
 */
/*--------------------------------------------------------------------------------*/
bool ADMData::SetAxml(const uint8_t *data, uint_t length)
{
  std::string str;

  str.assign((const char *)data, length);

  return SetAxml(str);
}

/*--------------------------------------------------------------------------------*/
/** Read ADM data from explicit XML
 *
 * @param data XML data stored as a string
 *
 * @return true if data read successfully
 */
/*--------------------------------------------------------------------------------*/
bool ADMData::SetAxml(const std::string& data)
{
  bool success = false;

  DEBUG3(("Read XML:\n%s", data.c_str()));

  if (TranslateXML(data))
  {
    ConnectReferences();

    success = true;
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Read ADM data from the chna and axml RIFF chunks
 *
 * @param chna ptr to chna chunk data 
 * @param chnalength length of chna data
 * @param axml ptr to axml chunk data 
 * @param axmllength length of axml data
 *
 * @return true if data read successfully
 */
/*--------------------------------------------------------------------------------*/
bool ADMData::Set(const uint8_t *chna, uint_t chnalength, const uint8_t *axml, uint_t axmllength)
{
  return (SetChna(chna, chnalength) && SetAxml(axml, axmllength));
}

/*--------------------------------------------------------------------------------*/
/** Create chna chunk data
 *
 * @param len reference to length variable to be updated with the size of the chunk
 *
 * @return ptr to chunk data
 */
/*--------------------------------------------------------------------------------*/
uint8_t *ADMData::GetChna(uint32_t& len) const
{
  CHNA_CHUNK *p = NULL;

  // calculate size of chna chunk
  len = sizeof(*p) + tracklist.size() * sizeof(p->UIDs[0]);
  if ((p = (CHNA_CHUNK *)calloc(1, len)) != NULL)
  {
    std::vector<bool> uniquetracks; // list of unique tracks
    uint_t i;
    
    // allocate maximum number of tracks
    uniquetracks.resize(tracklist.size());
    
    // populate structure
    p->TrackCount = 0;
    p->UIDCount   = 0;
    
    for (i = 0; i < tracklist.size(); i++)
    {
      const ADMAudioTrack *track = tracklist[i];
      uint_t tr = track->GetTrackNum();

      // test to see if uniquetracks array needs expanding
      if (tr >= uniquetracks.size()) uniquetracks.resize(tr + 1);
      
      // if this is a new track (i.e. one not previously used),increment TrackCount
      if (!uniquetracks[tr])
      {
        // set flag
        uniquetracks[tr] = true;
        // increment TrackCount
        p->TrackCount++;
      }

      // set track number (1- based) and UID
      p->UIDs[i].TrackNum = tr + 1;
      strncpy(p->UIDs[i].UID, track->GetID().c_str(), sizeof(p->UIDs[i].UID));

      // set trackformat references
      const ADMAudioTrackFormat *trackref = NULL;
      if (track->GetTrackFormatRefs().size() && ((trackref = track->GetTrackFormatRefs()[0]) != NULL))
      {
        strncpy(p->UIDs[i].TrackRef, trackref->GetID().c_str(), sizeof(p->UIDs[i].TrackRef));
      }

      // set packformat references
      const ADMAudioPackFormat *packref = NULL;
      if (track->GetPackFormatRefs().size() && ((packref = track->GetPackFormatRefs()[0]) != NULL))
      {
        strncpy(p->UIDs[i].PackRef, packref->GetID().c_str(), sizeof(p->UIDs[i].PackRef));
      }
        
      p->UIDCount++;

      DEBUG2(("Track %u/%u: Index %u UID '%s' TrackFormatRef '%s' PackFormatRef '%s'",
              i, (uint_t)tracklist.size(),
              track->GetTrackNum(),
              track->GetID().c_str(),
              trackref ? trackref->GetID().c_str() : "<none>",
              packref  ? packref->GetID().c_str()  : "<none>"));
    }
  }

  return (uint8_t *)p;
}

/*--------------------------------------------------------------------------------*/
/** Create axml chunk data
 *
 * @param indent indent string to use within XML
 * @param eol end of line string to use within XML
 * @param ind_level initial indentation level
 *
 * @return string containing XML data for axml chunk
 */
/*--------------------------------------------------------------------------------*/
std::string ADMData::GetAxml(const std::string& indent, const std::string& eol, uint_t ind_level) const
{
  std::string str;

  Printf(str,
         "%s<?xml version=\"1.0\" encoding=\"UTF-8\"?>%s",
         CreateIndent(indent, ind_level).c_str(), eol.c_str());

  Printf(str,
         "%s<ebuCoreMain xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns=\"urn:ebu:metadata-schema:ebuCore_2014\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" schema=\"EBU_CORE_20140201.xsd\" xml:lang=\"en\">%s",
         CreateIndent(indent, ind_level).c_str(), eol.c_str()); ind_level++;

  GenerateXML(str, indent, eol, ind_level);

  ind_level--;
  Printf(str,
         "%s</ebuCoreMain>%s",
         CreateIndent(indent, ind_level).c_str(), eol.c_str());

  DEBUG3(("Generated XML:\n%s", str.c_str()));

  return str;
}

/*--------------------------------------------------------------------------------*/
/** Create an ADM capable of decoding supplied XML as axml chunk
 */
/*--------------------------------------------------------------------------------*/
ADMData *ADMData::Create()
{
  ADMData *data = NULL;
  uint_t i;

  for (i = 0; i < providerlist.size(); i++)
  {
    const PROVIDER& provider = providerlist[i];

    if ((data = (*provider.fn)(provider.context)) != NULL) break;
  }

  return data;
}

/*--------------------------------------------------------------------------------*/
/** Register a provider for the above
 */
/*--------------------------------------------------------------------------------*/
void ADMData::RegisterProvider(CREATOR fn, void *context)
{
  PROVIDER provider =
  {
    .fn      = fn,
    .context = context,
  };

  providerlist.push_back(provider);
}

/*--------------------------------------------------------------------------------*/
/** Register an ADM sub-object with this ADM
 *
 * @param obj ptr to ADM object
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMData::Register(ADMObject *obj)
{
  const ADMAudioTrack *track;

  admobjects[obj->GetMapEntryID()] = obj;

  // if object is an audioTrack object, add it to the tracklist (list will be sorted later)
  if ((track = dynamic_cast<const ADMAudioTrack *>(obj)) != NULL)
  {
    tracklist.push_back(track);
  }

  obj->SetReferences();
}

bool ADMData::ValidType(const std::string& type) const
{
  return ((type == ADMAudioProgramme::Type) ||
          (type == ADMAudioContent::Type) ||
          (type == ADMAudioObject::Type) ||
          (type == ADMAudioPackFormat::Type) ||
          (type == ADMAudioBlockFormat::Type) ||
          (type == ADMAudioChannelFormat::Type) ||
          (type == ADMAudioStreamFormat::Type) ||
          (type == ADMAudioTrackFormat::Type) ||
          (type == ADMAudioTrack::Type));
}

/*--------------------------------------------------------------------------------*/
/** Create an ADM sub-object within this ADM object
 *
 * @param type object type - should always be the static 'Type' member of the object to be created (e.g. ADMAudioProgramme::Type)
 * @param id unique ID for the object (or empty string to create one using CreateID())
 * @param name human-readable name of the object
 *
 * @return ptr to object or NULL if type unrecognized or the object already exists
 */
/*--------------------------------------------------------------------------------*/
ADMObject *ADMData::Create(const std::string& type, const std::string& id, const std::string& name)
{
  ADMObject *obj = NULL;

  if (ValidType(type))
  {
    ADMOBJECTS_CIT it;
    // if id is empty, create one
    std::string uuid = type + "/" + ((id != "") ? id : CreateID(type));

    // ensure the id doesn't already exist
    if ((it = admobjects.find(uuid)) == admobjects.end())
    {
      if      (type == ADMAudioProgramme::Type)     obj = new ADMAudioProgramme(*this, id, name);
      else if (type == ADMAudioContent::Type)       obj = new ADMAudioContent(*this, id, name);
      else if (type == ADMAudioObject::Type)        obj = new ADMAudioObject(*this, id, name);
      else if (type == ADMAudioPackFormat::Type)    obj = new ADMAudioPackFormat(*this, id, name);
      else if (type == ADMAudioBlockFormat::Type)   obj = new ADMAudioBlockFormat(*this, id, name);
      else if (type == ADMAudioChannelFormat::Type) obj = new ADMAudioChannelFormat(*this, id, name);
      else if (type == ADMAudioStreamFormat::Type)  obj = new ADMAudioStreamFormat(*this, id, name);
      else if (type == ADMAudioTrackFormat::Type)   obj = new ADMAudioTrackFormat(*this, id, name);
      else if (type == ADMAudioTrack::Type)         obj = new ADMAudioTrack(*this, id, name);
    }
    else obj = it->second;
  }

  return obj;
}

/*--------------------------------------------------------------------------------*/
/** Find an unique ID given the specified format string
 *
 * @param type object type
 * @param format C-style format string
 * @param start starting index
 * 
 * @return unique ID
 */
/*--------------------------------------------------------------------------------*/
std::string ADMData::FindUniqueID(const std::string& type, const std::string& format, uint_t start) const
{
  ADMOBJECTS_CIT it;
  std::string id;
  uint_t n = start;

  // increment test value until ID is unique
  while (true)
  {
    std::string testid;
      
    Printf(testid, format.c_str(), ++n);

    // test this ID
    if ((it = admobjects.find(type + "/" + testid)) == admobjects.end())
    {
      // ID not already in list -> must be unique
      id = testid;
      break;
    }
  }

  return id;
}

/*--------------------------------------------------------------------------------*/
/** Create an unique ID (temporary) for the specified object
 *
 * @param type object type string
 *
 * @return unique ID
 *
 * @note in some cases, this ID is TEMPORARY and will be updated by the object itself
 */
/*--------------------------------------------------------------------------------*/
std::string ADMData::CreateID(const std::string& type) const
{
  std::string id;

  if (ValidType(type))
  {
    std::string format;
    uint_t start = 0;

    if      (type == ADMAudioProgramme::Type)     {format = ADMAudioProgramme::IDPrefix    + "%04x"; start = 0x1000;}
    else if (type == ADMAudioContent::Type)       {format = ADMAudioContent::IDPrefix      + "%04x"; start = 0x1000;}
    else if (type == ADMAudioObject::Type)        {format = ADMAudioObject::IDPrefix       + "%04x"; start = 0x1000;}
    else if (type == ADMAudioPackFormat::Type)    format = ADMAudioPackFormat::IDPrefix    + "%08u_T";    // temporary
    else if (type == ADMAudioBlockFormat::Type)   format = ADMAudioBlockFormat::IDPrefix   + "%08u_T";    // temporary
    else if (type == ADMAudioChannelFormat::Type) format = ADMAudioChannelFormat::IDPrefix + "%08u_T";    // temporary
    else if (type == ADMAudioStreamFormat::Type)  format = ADMAudioStreamFormat::IDPrefix  + "%08u_T";    // temporary
    else if (type == ADMAudioTrackFormat::Type)   format = ADMAudioTrackFormat::IDPrefix   + "%08u_T";    // temporary
    else if (type == ADMAudioTrack::Type)         format = ADMAudioTrack::IDPrefix         + "%08u_T";    // temporary

    id = FindUniqueID(type, format, start);
  }

  return id;
}

/*--------------------------------------------------------------------------------*/
/** Change the ID of the specified object
 *
 * @param obj ADMObject to change ID of
 * @param id new ID
 * @param start starting index used for search
 */
/*--------------------------------------------------------------------------------*/
void ADMData::ChangeID(ADMObject *obj, const std::string& id, uint_t start)
{
  // detect format characters in id
  bool format = (id.find("%") != std::string::npos);

  // if ID is to be updated, remove existing object from map
  // and re-enter with its new ID
  if (format || (id != obj->GetID()))
  {
    ADMOBJECTS_IT it;
  
    // find object in map and delete it
    if ((it = admobjects.find(obj->GetMapEntryID())) != admobjects.end())
    {
      admobjects.erase(it);
    }

    // if id is a format string, find unique ID
    if (format) obj->SetUpdatedID(FindUniqueID(obj->GetType(), id, start));
    // else just update object's ID
    else        obj->SetUpdatedID(id);

    // put object back into map with new ID
    admobjects[obj->GetMapEntryID()] = obj;
  }
}

/*--------------------------------------------------------------------------------*/
/** Create audioProgramme object
 *
 * @param name name of object
 *
 * @note ID will be create automatically
 *
 * @return ADMAudioProgramme object
 */
/*--------------------------------------------------------------------------------*/
ADMAudioProgramme *ADMData::CreateProgramme(const std::string& name)
{
  return new ADMAudioProgramme(*this, CreateID(ADMAudioProgramme::Type), name);
}

/*--------------------------------------------------------------------------------*/
/** Create audioContent object
 *
 * @param name name of object
 * @param programme audioProgramme object to attach this object to or NULL
 *
 * @note ID will be create automatically
 *
 * @return ADMAudioContent object
 */
/*--------------------------------------------------------------------------------*/
ADMAudioContent *ADMData::CreateContent(const std::string& name, ADMAudioProgramme *programme)
{
  ADMAudioContent *content;

  if ((content = new ADMAudioContent(*this, CreateID(ADMAudioContent::Type), name)) != NULL)
  {
    if (programme) programme->Add(content);
  }

  return content;
}

/*--------------------------------------------------------------------------------*/
/** Create audioObject object
 *
 * @param name name of object
 * @param content audioContent object to attach this object to or NULL
 *
 * @note ID will be create automatically
 *
 * @return ADMAudioObject object
 */
/*--------------------------------------------------------------------------------*/
ADMAudioObject *ADMData::CreateObject(const std::string& name, ADMAudioContent *content)
{
  ADMAudioObject *object;

  if ((object = new ADMAudioObject(*this, CreateID(ADMAudioObject::Type), name)) != NULL)
  {
    if (content) content->Add(object);
  }

  return object;
}

/*--------------------------------------------------------------------------------*/
/** Create audioPackFormat object
 *
 * @param name name of object
 * @param object audioObject object to attach this object to or NULL
 *
 * @note ID will be create automatically
 *
 * @return ADMAudioPackFormat object
 */
/*--------------------------------------------------------------------------------*/
ADMAudioPackFormat *ADMData::CreatePackFormat(const std::string& name, ADMAudioObject *object)
{
  ADMAudioPackFormat *packFormat;

  if ((packFormat = new ADMAudioPackFormat(*this, CreateID(ADMAudioPackFormat::Type), name)) != NULL)
  {
    if (object) object->Add(packFormat);
  }

  return packFormat;
}

/*--------------------------------------------------------------------------------*/
/** Create audioTrack object
 *
 * @param name name of object
 * @param object audioObject object to attach this object to or NULL
 *
 * @note ID will be create automatically
 *
 * @return ADMAudioTrack object
 */
/*--------------------------------------------------------------------------------*/
ADMAudioTrack *ADMData::CreateTrack(const std::string& name, ADMAudioObject *object)
{
  ADMAudioTrack *track;

  if ((track = new ADMAudioTrack(*this, CreateID(ADMAudioTrack::Type), name)) != NULL)
  {
    if (object) object->Add(track);
  }

  return track;
}

/*--------------------------------------------------------------------------------*/
/** Create audioChannelFormat object
 *
 * @param name name of object
 * @param packFormat audioPackFormat object to attach this object to or NULL
 * @param streamFormat audioStreamFormat object to attach this object to or NULL
 *
 * @note ID will be create automatically
 *
 * @return ADMAudioChannelFormat object
 */
/*--------------------------------------------------------------------------------*/
ADMAudioChannelFormat *ADMData::CreateChannelFormat(const std::string& name, ADMAudioPackFormat *packFormat, ADMAudioStreamFormat *streamFormat)
{
  ADMAudioChannelFormat *channelFormat;

  if ((channelFormat = new ADMAudioChannelFormat(*this, CreateID(ADMAudioChannelFormat::Type), name)) != NULL)
  {
    if (packFormat) packFormat->Add(channelFormat);
    if (streamFormat) streamFormat->Add(channelFormat);
  }

  return channelFormat;
}

/*--------------------------------------------------------------------------------*/
/** Create audioBlockFormat object
 *
 * @param name name of object
 * @param channelFormat audioChannelFormat object to attach this object to or NULL
 *
 * @note ID will be create automatically
 *
 * @return ADMAudioBlockFormat object
 */
/*--------------------------------------------------------------------------------*/
ADMAudioBlockFormat *ADMData::CreateBlockFormat(const std::string& name, ADMAudioChannelFormat *channelFormat)
{
  ADMAudioBlockFormat *blockFormat;

  if ((blockFormat = new ADMAudioBlockFormat(*this, CreateID(ADMAudioBlockFormat::Type), name)) != NULL)
  {
    if (channelFormat) channelFormat->Add(blockFormat);
  }

  return blockFormat;
}

/*--------------------------------------------------------------------------------*/
/** Create audioTrackFormat object
 *
 * @param name name of object
 * @param streamFormat audioStreamFormat object to attach this object to or NULL
 *
 * @note ID will be create automatically
 *
 * @return ADMAudioTrackFormat object
 */
/*--------------------------------------------------------------------------------*/
ADMAudioTrackFormat *ADMData::CreateTrackFormat(const std::string& name, ADMAudioStreamFormat *streamFormat)
{
  ADMAudioTrackFormat *trackFormat;

  if ((trackFormat = new ADMAudioTrackFormat(*this, CreateID(ADMAudioTrackFormat::Type), name)) != NULL)
  {
    if (streamFormat)
    {
      streamFormat->Add(trackFormat);
      trackFormat->Add(streamFormat);
    }
  }

  return trackFormat;
}

/*--------------------------------------------------------------------------------*/
/** Create audioStreamFormat object
 *
 * @param name name of object
 * @param trackFormat audioTrackFormat object to attach this object to or NULL
 *
 * @note ID will be create automatically
 *
 * @return ADMAudioStreamFormat object
 */
/*--------------------------------------------------------------------------------*/
ADMAudioStreamFormat *ADMData::CreateStreamFormat(const std::string& name, ADMAudioTrackFormat *trackFormat)
{
  ADMAudioStreamFormat *streamFormat;

  if ((streamFormat = new ADMAudioStreamFormat(*this, CreateID(ADMAudioStreamFormat::Type), name)) != NULL)
  {
    if (trackFormat)
    {
      trackFormat->Add(streamFormat);
      streamFormat->Add(trackFormat);
    }
  }

  return streamFormat;
}

/*--------------------------------------------------------------------------------*/
/** Parse XML section
 *
 * @param type object type
 * @param userdata context data
 *
 * @return newly created object
 */
/*--------------------------------------------------------------------------------*/
ADMObject *ADMData::Parse(const std::string& type, void *userdata)
{
  ADMHEADER header;
  ADMObject *obj;

  ParseHeader(header, type, userdata);

  if ((obj = Create(type, header.id, header.name)) != NULL)
  {
    ParseValues(obj, type, userdata);
    PostParse(obj, type, userdata);

    obj->SetValues();
  }

  return obj;
}

/*--------------------------------------------------------------------------------*/
/** Return the object associated with the specified reference
 *
 * @param value a name/value pair specifying object type and name
 */
/*--------------------------------------------------------------------------------*/
ADMObject *ADMData::GetReference(const ADMVALUE& value)
{
  ADMObject *obj = NULL;
  ADMOBJECTS_CIT it;
  std::string uuid = value.name, cmp;

  cmp = "UIDRef";
  if ((uuid.size() >= cmp.size()) && (uuid.compare(uuid.size() - cmp.size(), cmp.size(), cmp) == 0))
  {
    uuid = uuid.substr(0, uuid.size() - 3);
  }
  else
  {
    cmp = "IDRef";
    if ((uuid.size() >= cmp.size()) && (uuid.compare(uuid.size() - cmp.size(), cmp.size(), cmp) == 0))
    {
      uuid = uuid.substr(0, uuid.size() - cmp.size());
    }
  }

  uuid += "/" + value.value;

  if ((it = admobjects.find(uuid)) != admobjects.end()) obj = it->second;
  else
  {
#if DEBUG_LEVEL >= 4
    DEBUG1(("Failed to find reference '%s', object list:", uuid.c_str()));
    for (it = admobjects.begin(); it != admobjects.end(); ++it)
    {
      DEBUG1(("\t%s / %s (%u / %u)%s", it->first.c_str(), uuid.c_str(), (uint_t)it->first.size(), (uint_t)uuid.size(), (it->first == uuid) ? " *" : ""));
    }
#endif
  }

  return obj;
}

/*--------------------------------------------------------------------------------*/
/** Sort tracks into numerical order
 */
/*--------------------------------------------------------------------------------*/
void ADMData::SortTracks()
{
  sort(tracklist.begin(), tracklist.end(), ADMAudioTrack::Compare);

#if DEBUG_LEVEL >= 4
  std::vector<const ADMAudioTrack *>::const_iterator it;

  DEBUG1(("%lu tracks:", tracklist.size()));
  for (it = tracklist.begin(); it != tracklist.end(); ++it)
  {
    DEBUG1(("%u: %s", (*it)->GetTrackNum(), (*it)->ToString().c_str()));
  }
#endif
}

/*--------------------------------------------------------------------------------*/
/** Connect references between objects
 */
/*--------------------------------------------------------------------------------*/
void ADMData::ConnectReferences()
{
  ADMOBJECTS_IT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    it->second->SetReferences();
  }
}

/*--------------------------------------------------------------------------------*/
/** Get list of objects of specified type
 *
 * @param type audioXXX object type
 * @param list list to be populated
 */
/*--------------------------------------------------------------------------------*/
void ADMData::GetADMList(const std::string& type, std::vector<const ADMObject *>& list) const
{
  ADMOBJECTS_CIT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    const ADMObject *obj = it->second;

    if (obj->GetType() == type)
    {
      list.push_back(obj);
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Get ADM object by ID (with optional object type specified)
 *
 * @return object or NULL
 */
/*--------------------------------------------------------------------------------*/
const ADMObject *ADMData::GetObjectByID(const std::string& id, const std::string& type) const
{
  ADMOBJECTS_CIT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    const ADMObject *obj = it->second;

    if (((type == "") || (obj->GetType() == type)) && (obj->GetID() == id)) return obj;
  }

  return NULL;
}

/*--------------------------------------------------------------------------------*/
/** Get ADM object by Name (with optional object type specified)
 *
 * @return object or NULL
 */
/*--------------------------------------------------------------------------------*/
const ADMObject *ADMData::GetObjectByName(const std::string& name, const std::string& type) const
{
  ADMOBJECTS_CIT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    const ADMObject *obj = it->second;

    if (((type == "") || (obj->GetType() == type)) && (obj->GetName() == name)) return obj;
  }

  return NULL;
}

/*--------------------------------------------------------------------------------*/
/** Get writable ADM object by ID (with optional object type specified)
 *
 * @return object or NULL
 */
/*--------------------------------------------------------------------------------*/
ADMObject *ADMData::GetWritableObjectByID(const std::string& id, const std::string& type)
{
  ADMOBJECTS_IT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    ADMObject *obj = it->second;

    if (((type == "") || (obj->GetType() == type)) && (obj->GetID() == id)) return obj;
  }

  return NULL;
}

/*--------------------------------------------------------------------------------*/
/** Get writable ADM object by Name (with optional object type specified)
 *
 * @return object or NULL
 */
/*--------------------------------------------------------------------------------*/
ADMObject *ADMData::GetWritableObjectByName(const std::string& name, const std::string& type)
{
  ADMOBJECTS_IT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    ADMObject *obj = it->second;

    if (((type == "") || (obj->GetType() == type)) && (obj->GetName() == name)) return obj;
  }

  return NULL;
}


/*--------------------------------------------------------------------------------*/
/** Return a list of all ADM Audio Objects
 */
/*--------------------------------------------------------------------------------*/
void ADMData::GetAudioObjectList(std::vector<const ADMAudioObject *>& list) const
{
  ADMOBJECTS_CIT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    const ADMAudioObject *obj;

    if ((obj = dynamic_cast<const ADMAudioObject *>(it->second)) != NULL) list.push_back(obj);
  }
}

/*--------------------------------------------------------------------------------*/
/** Dump ADM or part of ADM as textual description
 *
 * @param str std::string to be modified with description
 * @param obj ADM object or NULL to dump the entire ADM
 * @param indent indentation for each level of objects
 * @param eol end-of-line string
 * @param level initial indentation level
 */
/*--------------------------------------------------------------------------------*/
void ADMData::Dump(std::string& str, const ADMObject *obj, const std::string& indent, const std::string& eol, uint_t level) const
{
  std::map<const ADMObject *,bool> map;
  DUMPCONTEXT    context;
  ADMOBJECTS_CIT it;

  // initialise context
  context.str       = str;
  context.indent    = indent;
  context.eol       = eol;
  context.ind_level = level;

  // if explicit object specified, dump it
  if (obj) Dump(obj, map, context);
  else
  {
    // otherwise find Programme object and start with it
    for (it = admobjects.begin(); it != admobjects.end(); ++it)
    {
      const ADMObject *obj = it->second;

      if (obj->GetType() == ADMAudioProgramme::Type)
      {
        Dump(obj, map, context);
      }
    }
  }

  // return generated string
  str = context.str;
}

/*--------------------------------------------------------------------------------*/
/** Generate textual description of ADM object (recursive
 *
 * @param obj ADM object
 * @param map map of objects already stored (bool is a dummy)
 * @param context DUMPCONTEXT for tracking indentation, etc
 */
/*--------------------------------------------------------------------------------*/
void ADMData::Dump(const ADMObject *obj, std::map<const ADMObject *,bool>& map, DUMPCONTEXT& context) const
{
  std::vector<ADMObject::REFERENCEDOBJECT> objects;
  ADMObject::ADMVALUES values;
  std::string& str = context.str;
  std::string  indent; 
  uint_t i;

  // mark this object is generated
  map[obj] = true;

  obj->GetValuesAndReferences(values, objects, true);

  indent = CreateIndent(context.indent, context.ind_level++);

  Printf(str, "%s%s: %s", indent.c_str(), obj->GetType().c_str(), obj->GetID().c_str());

  if (obj->GetName() != "")
  {
    Printf(str, " / %s", obj->GetName().c_str());
  }
  
  str    += context.eol;
  indent += context.indent;

  // output attributes
  for (i = 0; i < values.size(); i++)
  {
    const ADMVALUE& value = values[i];

    if (value.attr)
    {
      Printf(str, "%s%s: %s%s", indent.c_str(), value.name.c_str(), value.value.c_str(), context.eol.c_str());
    }
  }
  
  // output values
  for (i = 0; i < values.size(); i++)
  {
    const ADMVALUE& value = values[i];

    if (!value.attr)
    {
      ADMObject::ADMATTRS::const_iterator it;

      Printf(str, "%s%s", indent.c_str(), value.name.c_str());

      if (value.attrs.end() != value.attrs.begin())
      {
        Printf(str, " (");
        for (it = value.attrs.begin(); it != value.attrs.end(); ++it)
        {
          Printf(str, "%s=%s", it->first.c_str(), it->second.c_str());
        }
        Printf(str, ")");
      }

      Printf(str, ": %s%s", value.value.c_str(), context.eol.c_str());
    }
  }

  // output references
  for (i = 0; i < objects.size(); i++)
  {
    const ADMObject::REFERENCEDOBJECT& object = objects[i];
    bool dumpthisobject = (map.find(object.obj) == map.end());

    if (object.genref)
    {
      // output reference to object.obj
      Printf(str, "%s%s: %s", indent.c_str(), object.obj->GetReference().c_str(), object.obj->GetID().c_str());

      if (dumpthisobject)
      {
        str += context.eol;
        context.ind_level++;
        Dump(object.obj, map, context);
        context.ind_level--;
      }
      else Printf(str, " (see above)%s", context.eol.c_str());
    }
    else if (dumpthisobject) Dump(object.obj, map, context);
  }

  context.ind_level--;
}

/*--------------------------------------------------------------------------------*/
/** Start XML
 *
 * @param xmlcontext user supplied argument representing context data
 * @param version XML version
 * @param encoding XML encoding
 *
 * @note for other XML implementaions, this function MUST be overridden
 */
/*--------------------------------------------------------------------------------*/
void ADMData::StartXML(void *xmlcontext, const std::string& version, const std::string& encoding) const
{
  TEXTXML& xml = *(TEXTXML *)xmlcontext;

  Printf(xml.str,
         "%s<?xml version=\"%s\" encoding=\"%s\"?>%s",
         CreateIndent(xml.indent, xml.ind_level).c_str(),
         version.c_str(),
         encoding.c_str(),
         xml.eol.c_str());
}

/*--------------------------------------------------------------------------------*/
/** Start an XML object
 *
 * @param xmlcontext user supplied argument representing context data
 * @param name object name
 *
 * @note for other XML implementaions, this function MUST be overridden
 */
/*--------------------------------------------------------------------------------*/
void ADMData::OpenXMLObject(void *xmlcontext, const std::string& name) const
{
  TEXTXML& xml = *(TEXTXML *)xmlcontext;

  // ensure any previous object is properly marked ready for data
  if (xml.stack.size() && xml.opened)
  {
    xml.str   += ">";
    xml.opened = false;
  }

  // add a newline if last bit of string isn't an eol
  if (xml.eol.length() &&
      (xml.str.length() >= xml.eol.length()) &&
      (xml.str.substr(xml.str.length() - xml.eol.length()) != xml.eol)) xml.str += xml.eol;

  Printf(xml.str,
         "%s<%s",
         CreateIndent(xml.indent, xml.ind_level + xml.stack.size()).c_str(),
         name.c_str()); 

  // stack this object name (for closing)
  xml.stack.push_back(name);
  xml.opened = true;
}

/*--------------------------------------------------------------------------------*/
/** Add an attribute to the current XML object
 *
 * @param xmlcontext user supplied argument representing context data
 * @param name attribute name
 * @param name attribute value
 *
 * @note for other XML implementaions, this function MUST be overridden
 */
/*--------------------------------------------------------------------------------*/
void ADMData::AddXMLAttribute(void *xmlcontext, const std::string& name, const std::string& value) const
{
  TEXTXML& xml = *(TEXTXML *)xmlcontext;

  Printf(xml.str, " %s=\"%s\"", name.c_str(), value.c_str());
}

/*--------------------------------------------------------------------------------*/
/** Set XML data
 *
 * @param xmlcontext user supplied argument representing context data
 * @param data data
 *
 * @note for other XML implementaions, this function MUST be overridden
 */
/*--------------------------------------------------------------------------------*/
void ADMData::SetXMLData(void *xmlcontext, const std::string& data) const
{
  TEXTXML& xml = *(TEXTXML *)xmlcontext;

  // ensure any object is marked ready for data
  if (xml.stack.size() && xml.opened)
  {
    xml.str   += ">";
    xml.opened = false;
  }

  xml.str += data;
}

/*--------------------------------------------------------------------------------*/
/** Close XML object
 *
 * @param xmlcontext user supplied argument representing context data
 *
 * @note for other XML implementaions, this function MUST be overridden
 */
/*--------------------------------------------------------------------------------*/
void ADMData::CloseXMLObject(void *xmlcontext) const
{
  TEXTXML& xml = *(TEXTXML *)xmlcontext;

  if (xml.stack.size() && xml.opened)
  {
    // object is empty
    xml.str   += " />" + xml.eol;
    xml.opened = false;
  }
  else
  {
    if ((xml.str.length() >= xml.eol.length()) && (xml.str.substr(xml.str.length() - xml.eol.length()) == xml.eol)) xml.str += CreateIndent(xml.indent, xml.ind_level + xml.stack.size() - 1);
    Printf(xml.str, "</%s>%s", xml.stack.back().c_str(), xml.eol.c_str());
  }

  xml.stack.pop_back();
}

/*--------------------------------------------------------------------------------*/
/** Create XML representation of ADM
 *
 * @param str std::string to be modified with XML
 * @param indent indentation for each level of objects
 * @param eol end-of-line string
 * @param level initial indentation level
 *
 * @note for other XML implementaions, this function can be overridden
 */
/*--------------------------------------------------------------------------------*/
void ADMData::GenerateXML(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const
{
  TEXTXML context;

  context.indent    = indent;
  context.eol       = eol;
  context.ind_level = ind_level;

  GenerateXML((void *)&context);

  str = context.str;
}

/*--------------------------------------------------------------------------------*/
/** Generic XML creation
 *
 * @param xmlcontext user supplied argument representing context data
 *
 * @note for other XML implementaions, this function can be overridden
 */
/*--------------------------------------------------------------------------------*/
void ADMData::GenerateXML(void *xmlcontext) const
{
  std::vector<const ADMObject *>   list;
  std::map<const ADMObject *,bool> map;
  ADMOBJECTS_CIT it;
  uint_t i;
  
  StartXML(xmlcontext);
  OpenXMLObject(xmlcontext, "ebuCoreMain");
  AddXMLAttribute(xmlcontext, "xmlns:dc", "http://purl.org/dc/elements/1.1/");
  AddXMLAttribute(xmlcontext, "xmlns", "urn:ebu:metadata-schema:ebuCore_2014");
  AddXMLAttribute(xmlcontext, "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
  AddXMLAttribute(xmlcontext, "schema", "EBU_CORE_20140201.xsd");
  AddXMLAttribute(xmlcontext, "xml:lang", "en");
  OpenXMLObject(xmlcontext, "coreMetadata");
  OpenXMLObject(xmlcontext, "format");
  OpenXMLObject(xmlcontext, "audioFormatExtended");

  // find Programme object and start with it
  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    const ADMObject *obj = it->second;

    if (obj->GetType() == ADMAudioProgramme::Type)
    {
      map[obj] = true;
      GenerateXML(obj, list, xmlcontext);
    }
  }

  // output referenced and child objects if they have been done so yet
  for (i = 0; i < list.size(); i++)
  {
    const ADMObject *obj = list[i];

    // has object been output?
    if (map.find(obj) == map.end())
    {
      // no, output it
      map[obj] = true;
      GenerateXML(obj, list, xmlcontext);
    }
  }

  CloseXMLObject(xmlcontext);
  CloseXMLObject(xmlcontext);
  CloseXMLObject(xmlcontext);
  CloseXMLObject(xmlcontext);
}

/*--------------------------------------------------------------------------------*/
/** Generic XML creation
 *
 * @param obj ADM object to generate XML for
 * @param list list of ADM objects that will subsequently need XML generating
 * @param xmlcontext user supplied argument representing context data
 *
 * @note for other XML implementaions, this function can be overridden
 */
/*--------------------------------------------------------------------------------*/
void ADMData::GenerateXML(const ADMObject *obj, std::vector<const ADMObject *>& list, void *xmlcontext) const
{
  std::vector<ADMObject::REFERENCEDOBJECT> objects;
  ADMObject::ADMVALUES values;
  uint_t i;
  bool   emptyobject = true;

  obj->GetValuesAndReferences(values, objects);

  // test to see if this object is 'empty'
  for (i = 0; emptyobject && (i < values.size()); i++)
  {
    emptyobject &= !(!values[i].attr);                          // if any values (non-attribute) found, object cannot be empty
  }
  for (i = 0; emptyobject && (i < objects.size()); i++)
  {
    emptyobject &= !(objects[i].genref || objects[i].gendata);  // if any references or contained data to be generated, object cannot be empty
  }

  // start XML object
  OpenXMLObject(xmlcontext, obj->GetType());

  // output attributes
  for (i = 0; i < values.size(); i++)
  {
    const ADMVALUE& value = values[i];

    if (value.attr)
    {
      AddXMLAttribute(xmlcontext, value.name, value.value);
    }
  }
  
  if (!emptyobject)
  {
    // output values
    for (i = 0; i < values.size(); i++)
    {
      const ADMVALUE& value = values[i];

      if (!value.attr)
      {
        ADMObject::ADMATTRS::const_iterator it;

        OpenXMLObject(xmlcontext, value.name);

        for (it = value.attrs.begin(); it != value.attrs.end(); ++it)
        {
          AddXMLAttribute(xmlcontext, it->first, it->second);
        }

        SetXMLData(xmlcontext, value.value);
        
        CloseXMLObject(xmlcontext);
      }
    }

    // output references
    for (i = 0; i < objects.size(); i++)
    {
      const ADMObject::REFERENCEDOBJECT& object = objects[i];

      if (object.genref)
      {
        // output reference to object.obj
        OpenXMLObject(xmlcontext, object.obj->GetReference());
        SetXMLData(xmlcontext, object.obj->GetID());
        CloseXMLObject(xmlcontext);
      }
    }

    // output contained data
    for (i = 0; i < objects.size(); i++)
    {
      const ADMObject::REFERENCEDOBJECT& object = objects[i];

      if (object.gendata)
      {
        GenerateXML(object.obj, list, xmlcontext);
      }
      else
      {
        // object itself needs to be output
        list.push_back(object.obj);
      }
    }

    // end XML object
    CloseXMLObject(xmlcontext);
  }
  else
  {
    // end empty XML object
    CloseXMLObject(xmlcontext);

    // empty object but need to add objects to list to be processed
    for (i = 0; i < objects.size(); i++)
    {
      // object itself needs to be output
      list.push_back(objects[i].obj);
    }      
  }
}

/*--------------------------------------------------------------------------------*/
/** Generate a textual list of references 
 *
 * @param str string to be modified
 */
/*--------------------------------------------------------------------------------*/
void ADMData::GenerateReferenceList(std::string& str)
{
  ADMOBJECTS_CIT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    const ADMObject *obj = it->second;

    obj->GenerateReferenceList(str);
  }
}

/*--------------------------------------------------------------------------------*/
/** Create/link ADM objects
 *
 * @param data OBJECTNAMES structure populated with names of objects to link/create (empty names are not created/linked)
 *
 * @return true if successful
 */
/*--------------------------------------------------------------------------------*/
bool ADMData::CreateObjects(const OBJECTNAMES& names)
{
  ADMAudioProgramme     *programme     = NULL;
  ADMAudioContent       *content       = NULL;
  ADMAudioObject        *object        = NULL;
  ADMAudioPackFormat    *packFormat    = NULL;
  ADMAudioChannelFormat *channelFormat = NULL;
  ADMAudioStreamFormat  *streamFormat  = NULL;
  ADMAudioTrackFormat   *trackFormat   = NULL;
  ADMAudioTrack         *audioTrack    = NULL;
  bool success = true;

  // look up objects and create any that need creating
  if ((names.programmeName != "") && ((programme = dynamic_cast<ADMAudioProgramme *>(GetWritableObjectByName(names.programmeName, ADMAudioProgramme::Type))) == NULL))
  {
    if ((programme = CreateProgramme(names.programmeName)) != NULL)
    {
      DEBUG2(("Created programme '%s'", names.programmeName.c_str()));
    }
    else
    {
      ERROR("Failed to create programme '%s'", names.programmeName.c_str());
      success = false;
    }
  }

  if ((names.contentName != "") && ((content = dynamic_cast<ADMAudioContent *>(GetWritableObjectByName(names.contentName, ADMAudioContent::Type))) == NULL))
  {
    if ((content = CreateContent(names.contentName)) != NULL)
    {
      DEBUG2(("Created content '%s'", names.contentName.c_str()));
    }
    else
    {
      ERROR("Failed to create content '%s'", names.contentName.c_str());
      success = false;
    }
  }

  if ((names.objectName != "") && ((object = dynamic_cast<ADMAudioObject *>(GetWritableObjectByName(names.objectName, ADMAudioObject::Type))) == NULL))
  {
    if ((object = CreateObject(names.objectName)) != NULL)
    {
      DEBUG2(("Created object '%s'", names.objectName.c_str()));
    }
    else
    {
      ERROR("Failed to create object '%s'", names.objectName.c_str());
      success = false;
    }
  }

  if ((names.packFormatName != "") && ((packFormat = dynamic_cast<ADMAudioPackFormat *>(GetWritableObjectByName(names.packFormatName, ADMAudioPackFormat::Type))) == NULL))
  {
    if ((packFormat = CreatePackFormat(names.packFormatName)) != NULL)
    {
      // set pack type
      DEBUG2(("Created pack format '%s'", names.packFormatName.c_str()));
      packFormat->SetTypeLabel(ADMObject::TypeLabel_Objects);
    }
    else
    {
      ERROR("Failed to create packFormat '%s'", names.packFormatName.c_str());
      success = false;
    }
  }

  if ((names.channelFormatName != "") && ((channelFormat = dynamic_cast<ADMAudioChannelFormat *>(GetWritableObjectByName(names.channelFormatName, ADMAudioChannelFormat::Type))) == NULL))
  {
    if ((channelFormat = CreateChannelFormat(names.channelFormatName)) != NULL)
    {
      // set channel type
      DEBUG2(("Created channel format '%s'", names.channelFormatName.c_str()));
      channelFormat->SetTypeLabel(ADMObject::TypeLabel_Objects);
    }
    else
    {
      ERROR("Failed to create channelFormat '%s'", names.channelFormatName.c_str());
      success = false;
    }
  }

  if ((names.streamFormatName != "") && ((streamFormat = dynamic_cast<ADMAudioStreamFormat *>(GetWritableObjectByName(names.streamFormatName, ADMAudioStreamFormat::Type))) == NULL))
  {
    if ((streamFormat = CreateStreamFormat(names.streamFormatName)) != NULL)
    {
      // set stream type (PCM)
      DEBUG2(("Created stream format '%s'", names.streamFormatName.c_str()));
      streamFormat->SetTypeLabel(ADMObject::TypeLabel_Objects);
      streamFormat->SetFormatLabel(1);
      streamFormat->SetFormatDefinition("PCM");
    }
    else
    {
      ERROR("Failed to create streamFormat '%s'", names.streamFormatName.c_str());
      success = false;
    }
  }

  if ((names.trackFormatName != "") && ((trackFormat = dynamic_cast<ADMAudioTrackFormat *>(GetWritableObjectByName(names.trackFormatName, ADMAudioTrackFormat::Type))) == NULL))
  {
    if ((trackFormat = CreateTrackFormat(names.trackFormatName)) != NULL)
    {
      // set track type (PCM)
      DEBUG2(("Created track format '%s'", names.trackFormatName.c_str()));
      trackFormat->SetFormatLabel(1);
      trackFormat->SetFormatDefinition("PCM");
    }
    else
    {
      ERROR("Failed to create trackFormat '%s'", names.trackFormatName.c_str());
      success = false;
    }
  }

  if (names.trackNumber < tracklist.size())
  {
    if ((audioTrack = const_cast<ADMAudioTrack *>(tracklist[names.trackNumber])) != NULL)
    {
      DEBUG2(("Found track number %u (%u tracks)", names.trackNumber, (uint_t)tracklist.size()));
    }
    else
    {
      ERROR("Failed to find track number %u (%u tracks)", names.trackNumber, (uint_t)tracklist.size());
    }
  }

#define LINK(master,slave)                          \
  if (master && slave)                              \
  {                                                 \
    if (master->Add(slave))                         \
    {                                               \
      DEBUG2(("Connected %s '%s' to %s '%s'",       \
              slave->GetType().c_str(),             \
              slave->GetName().c_str(),             \
              master->GetType().c_str(),            \
              master->GetName().c_str()));          \
    }                                               \
    else                                            \
    {                                               \
      ERROR("Failed to connect %s '%s' to %s '%s'", \
            slave->GetType().c_str(),               \
            slave->GetName().c_str(),               \
            master->GetType().c_str(),              \
            master->GetName().c_str());             \
      success = false;                              \
    }                                               \
  }

  // link objects
  LINK(packFormat, channelFormat);
  LINK(trackFormat, streamFormat);
  LINK(streamFormat, trackFormat);
  LINK(streamFormat, channelFormat);
  LINK(audioTrack, trackFormat);
  LINK(audioTrack, packFormat);

  // add the object to the content
  LINK(content, object);
  // add the pack to the object
  LINK(object, packFormat);
  // add the track to the object
  LINK(object, audioTrack);

  LINK(content, object);

  LINK(programme, content);
  
  return success;
}

/*--------------------------------------------------------------------------------*/
/** Create ADM from a simple text file
 *
 * @param filename file describing the basic ADM layout
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
bool ADMData::CreateFromFile(const char *filename)
{
  EnhancedFile fp;
  bool success = false;

  if (fp.fopen(filename, "r"))
  {
    static char line[1024];
    ADMData::OBJECTNAMES names;
    uint_t ln = 1;

    success = true;

    while (fp.readline(line, sizeof(line) - 1) != EOF)
    {
      if (ln == 1)
      {
        // line 1: <programme-name>[:<content-name>]
        const char *p;

        // set names for programme and content objects
        // find optionally different content name
        if ((p = strchr(line, ':')) != NULL)
        {
          names.programmeName.assign(line, p - line);
          names.contentName.assign(p + 1);
        }
        else names.programmeName = names.contentName = line;
      }
      else
      {
        const char *p1, *p2;
        uint_t tr;

        if (sscanf(line, "%u", &tr) == 1)
        {
          if ((tr > 0) && (tr <= tracklist.size()))
          {
            if (((p1 = strchr(line, ':')) != NULL) && ((p2 = strchr(p1 + 1, ':')) != NULL))
            {
              std::string trackname;

              p1++;

              // set track number
              names.trackNumber = tr - 1;

              // set track name
              trackname.assign(p1, p2 - p1);

              // derive channel and stream names from track name
              names.channelFormatName = trackname;
              names.streamFormatName  = "PCM_" + trackname;
              names.trackFormatName   = "PCM_" + trackname;

              // set object name
              names.objectName.assign(p2 + 1);

              // set pack name from object name
              names.packFormatName = names.objectName;

              // create / connect objects
              CreateObjects(names);
            }
            else
            {
              ERROR("Failed to decode <tr>:<trackname>:<objectname> line");
              success = false;
            }
          }
          else
          {
            ERROR("Track %u out of range 1-%u", tr, (uint_t)tracklist.size());
            success = false;
          }
        }
        else
        {
          ERROR("Failed to extract track number from '%s'", line);
          success = false;
        }
      }

      ln++;
    }

    fp.fclose();
  }

  return success;
}

BBC_AUDIOTOOLBOX_END
