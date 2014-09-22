
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>

#define DEBUG_LEVEL 1
#include <aplibs-dsp/EnhancedFile.h>

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
 *
 * @return true if data read successfully
 */
/*--------------------------------------------------------------------------------*/
bool ADMData::SetChna(const uint8_t *data)
{
  const CHNA_CHUNK& chna = *(const CHNA_CHUNK *)data;
  std::string terminator;
  bool success = true;

  // create string with a single 0 byte in it to detect terminators
  terminator.push_back(0);

  uint16_t i;
  for (i = 0; i < chna.UIDCount; i++)
  {
    ADMAudioTrack *track;
    std::string id;

    id.assign(chna.UIDs[i].UID, sizeof(chna.UIDs[i].UID));

    if ((track = dynamic_cast<ADMAudioTrack *>(Create(ADMAudioTrack::Type, id, ""))) != NULL)
    {
      ADMVALUE value;

      value.attr = false;

      track->SetTrackNum(chna.UIDs[i].TrackNum);

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
    UpdateLimits();

    success = true;
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Read ADM data from the chna and axml RIFF chunks
 *
 * @param chna ptr to chna chunk data 
 * @param axml ptr to axml chunk data 
 * @param axmllength length of axml data
 *
 * @return true if data read successfully
 */
/*--------------------------------------------------------------------------------*/
bool ADMData::Set(const uint8_t *chna, const uint8_t *axml, uint_t axmllength)
{
  return (SetChna(chna) && SetAxml(axml, axmllength));
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

  len = sizeof(*p) + tracklist.size() * sizeof(p->UIDs[0]);
  if ((p = (CHNA_CHUNK *)calloc(1, len)) != NULL)
  {
    uint_t i;

    p->TrackCount = tracklist.size();
    p->UIDCount   = tracklist.size();
        
    for (i = 0; i < p->UIDCount; i++)
    {
      const ADMAudioTrack *track = tracklist[i];

      p->UIDs[i].TrackNum = track->GetTrackNum();
      strncpy(p->UIDs[i].UID, track->GetID().c_str(), sizeof(p->UIDs[i].UID));

      const ADMAudioTrackFormat *trackref = NULL;
      if (track->GetTrackFormatRefs().size() && ((trackref = track->GetTrackFormatRefs()[0]) != NULL))
      {
        strncpy(p->UIDs[i].TrackRef, trackref->GetID().c_str(), sizeof(p->UIDs[i].TrackRef));
      }

      const ADMAudioPackFormat *packref = NULL;
      if (track->GetPackFormatRefs().size() && ((packref = track->GetPackFormatRefs()[0]) != NULL))
      {
        strncpy(p->UIDs[i].PackRef, packref->GetID().c_str(), sizeof(p->UIDs[i].PackRef));
      }

      DEBUG2(("Track %u/%u: Index %u UID '%s' TrackFormatRef '%s' PackFormatRef '%s'",
              i + 1, p->UIDCount,
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
  std::string uuid = obj->GetType() + "/" + obj->GetID();

  admobjects[uuid] = obj;

  {
    const ADMAudioTrack *track;
    if ((track = dynamic_cast<const ADMAudioTrack *>(obj)) != NULL)
    {
      tracklist.push_back(track);
    }
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
ADMObject *ADMData::Create(const std::string& type, const std::string& id, const std::string& name, const ADMAudioChannelFormat *channelformat)
{
  ADMObject *obj = NULL;

  if (ValidType(type))
  {
    ADMOBJECTS_MAP::const_iterator it;
    // if id is empty, create one
    std::string uuid = type + "/" + ((id != "") ? id : CreateID(type, channelformat));

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
/** Create an unique ID for the specified type
 *
 * @param type object type - should always be the static 'Type' member of the object to be created (e.g. ADMAudioProgramme::Type)
 * @param channelformat ptr to channelformat object if type is ADMAudioBlockFormat::Type
 *
 * @return unique ID
 */
/*--------------------------------------------------------------------------------*/
std::string ADMData::CreateID(const std::string& type, const ADMAudioChannelFormat *channelformat) const
{
  std::string id;

  if (ValidType(type))
  {
    // create type dependant prefix
    if      (type == ADMAudioProgramme::Type)     id = "APR_";
    else if (type == ADMAudioContent::Type)       id = "ACO_";
    else if (type == ADMAudioObject::Type)        id = "AO_";
    else if (type == ADMAudioPackFormat::Type)    id = "AP_";
    else if (type == ADMAudioBlockFormat::Type)   id = "AB_";
    else if (type == ADMAudioChannelFormat::Type) id = "AC_";
    else if (type == ADMAudioStreamFormat::Type)  id = "AS_";
    else if (type == ADMAudioTrackFormat::Type)   id = "AT_";
    else if (type == ADMAudioTrack::Type)         id = "ATU_";

    // if type is an audioBlockFormat type (and a channel format is specified), append channel format's ID onto ID
    if ((type == ADMAudioBlockFormat::Type) && channelformat && (channelformat->GetID().substr(0, 3) == "AC_"))
    {
      id += channelformat->GetID().substr(3) + "_";
    }

    // find unique ID using existing map
    ADMOBJECTS_MAP::const_iterator it;
    uint_t n = 0;

    // for audioBlockFormat, the supplied channelformat object can give a good initial test value
    if ((type == ADMAudioBlockFormat::Type) && channelformat) n = channelformat->GetBlockFormatRefs().size();
    else
    {
      // count up objects of this type in admobjects
      for (it = admobjects.begin(); it != admobjects.end(); ++it)
      {
        // if object of same type, increment initial test value
        if (it->second->GetType() == type) n++;
      }
    }

    std::string format;
    // format for audioProgrammes, audioContents and audioObjects are slightly different (four digit ID vs eight digit ID)
    if ((type == ADMAudioProgramme::Type) ||
        (type == ADMAudioContent::Type)   ||
        (type == ADMAudioObject::Type))
    {
      format = "%s%04u";     // id_<num>
    }
    else
    {
      format = "%s%08u";     // id_<num>
    }

    // increment test value until ID is unique
    while (true)
    {
      std::string testid;
      
      Printf(testid, format.c_str(), id.c_str(), ++n);

      // test this ID
      if ((it = admobjects.find(type + "/" + testid)) == admobjects.end())
      {
        // ID not already in list -> must be unique
        id = testid;
        break;
      }
    }
  }

  DEBUG3(("Unique ID for type '%s' (channelformat '%s'): %s", type.c_str(), channelformat ? channelformat->ToString().c_str() : "", id.c_str()));

  return id;
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

void ADMData::SortTracks()
{
  std::vector<const ADMAudioTrack *>::const_iterator it;

  sort(tracklist.begin(), tracklist.end(), ADMAudioTrack::Compare);

  DEBUG4(("%lu tracks:", tracklist.size()));
  for (it = tracklist.begin(); it != tracklist.end(); ++it)
  {
    DEBUG4(("%u: %s", (*it)->GetTrackNum(), (*it)->ToString().c_str()));
  }
}

void ADMData::ConnectReferences()
{
  ADMOBJECTS_IT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    it->second->SetReferences();
  }
}

void ADMData::UpdateLimits()
{
  ADMOBJECTS_IT  it;
  ADMAudioObject *obj;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    if ((obj = dynamic_cast<ADMAudioObject *>(it->second)) != NULL)
    {
      obj->UpdateLimits();
    }
  }
}

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

std::string ADMData::FormatString(const char *fmt, ...)
{
  std::string str;
  va_list ap;

  va_start(ap, fmt);

  char *buf = NULL;
  if (vasprintf(&buf, fmt, ap) > 0)
  {
    str = buf;
    free(buf);
  }

  va_end(ap);

  return str;
}

void ADMData::Dump(std::string& str, const std::string& indent, const std::string& eol, uint_t level) const
{
  ADMOBJECTS_CIT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    if (it->second->GetType() == ADMAudioProgramme::Type)
    {
      it->second->Dump(str, indent, eol, level);
    }
  }
}

void ADMData::GenerateXML(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const
{
  ADMOBJECTS_CIT it;

  Printf(str,
         "%s<coreMetadata>%s",
         CreateIndent(indent, ind_level).c_str(), eol.c_str()); ind_level++;

  Printf(str,
         "%s<format>%s",
         CreateIndent(indent, ind_level).c_str(), eol.c_str()); ind_level++;

  Printf(str,
         "%s<audioFormatExtended>%s",
         CreateIndent(indent, ind_level).c_str(), eol.c_str()); ind_level++;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    const ADMObject *obj = it->second;

    if (obj->GetType() == ADMAudioProgramme::Type)
    {
      obj->GenerateXML(str, indent, eol, ind_level);
    }
  }

  ind_level--;
  Printf(str,
         "%s</audioFormatExtended>%s",
         CreateIndent(indent, ind_level).c_str(), eol.c_str());

  ind_level--;
  Printf(str,
         "%s</format>%s",
         CreateIndent(indent, ind_level).c_str(), eol.c_str());
    
  ind_level--;
  Printf(str,
         "%s</coreMetadata>%s",
         CreateIndent(indent, ind_level).c_str(), eol.c_str());
}

void ADMData::GenerateReferenceList(std::string& str)
{
  ADMOBJECTS_CIT it;

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    const ADMObject *obj = it->second;

    obj->GenerateReferenceList(str);
  }
}

void ADMData::CreateCursors(std::vector<PositionCursor *>& list, uint_t channel, uint_t nchannels) const
{
  uint_t i;

  channel   = MIN(channel,   tracklist.size() - 1);
  nchannels = MIN(nchannels, tracklist.size() - channel);

  for (i = 0; i < nchannels; i++)
  {
    list.push_back(new ADMTrackCursor(tracklist[channel + i]));
  }
}

void ADMData::Serialize(uint8_t *dst, uint_t& len) const
{
  ADMOBJECTS_CIT it;
  uint_t len0   = len;
  uint_t sublen = 0;

  if (dst) Serialize(NULL, sublen);

  ADMObject::SerializeData(dst, len, ADMObject::SerialDataType_ADMHeader, sublen);
  ADMObject::SerializeData(dst, len, (uint32_t)admobjects.size());
  ADMObject::SerializeSync(dst, len, len0);

  for (it = admobjects.begin(); it != admobjects.end(); ++it)
  {
    it->second->Serialize(dst, len);
  }

  ADMObject::SerializeObjectCRC(dst, len, len0);
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
    ADMAudioProgramme *programme = NULL;
    ADMAudioContent   *content   = NULL;
    uint_t ln = 1;

    success = true;

    while (fp.readline(line, sizeof(line) - 1) != EOF)
    {
      if (ln == 1)
      {
        // line 1: <programme-name>[:<content-name>]
        std::string programmename, contentname;
        const char *p;

        // find optionally different content name
        if ((p = strchr(line, ':')) != NULL)
        {
          programmename.assign(line, p - line);
          contentname.assign(p + 1);
        }
        else programmename = contentname = line;

        if ((programme = CreateProgramme(programmename)) != NULL)
        {
          if ((content = CreateContent(contentname, programme)) == NULL)
          {
            ERROR("Failed to create content '%s'", contentname.c_str());
            success = false;
          }
        }
        else
        {
          ERROR("Failed to create programme '%s'", programmename.c_str());
          success = false;
        }
      }
      else if (content)
      {
        const char *p1, *p2;
        uint_t tr;

        if (sscanf(line, "%u", &tr) == 1)
        {
          if ((tr > 0) && (tr <= tracklist.size()))
          {
            if (((p1 = strchr(line, ':')) != NULL) && ((p2 = strchr(p1 + 1, ':')) != NULL))
            {
              std::string trackname, objectname;

              p1++;
              
              trackname.assign(p1, p2 - p1);
              objectname.assign(p2 + 1);
              
              ADMAudioTrack         *audioTrack    = const_cast<ADMAudioTrack *>(tracklist[tr - 1]);
              ADMAudioPackFormat    *packFormat    = dynamic_cast<ADMAudioPackFormat *>(const_cast<ADMObject *>(GetObjectByName(objectname, ADMAudioPackFormat::Type)));
              ADMAudioChannelFormat *channelFormat = CreateChannelFormat(trackname);
              ADMAudioStreamFormat  *streamFormat  = CreateStreamFormat("PCM_" + trackname);
              ADMAudioTrackFormat   *trackFormat   = CreateTrackFormat("PCM_" + trackname);
              ADMAudioObject        *object        = dynamic_cast<ADMAudioObject *>(const_cast<ADMObject *>(GetObjectByName(objectname, ADMAudioObject::Type)));

              // the same pack can cover a number of tracks
              if (packFormat == NULL)
              {
                packFormat = CreatePackFormat(objectname);
                // set pack type
                packFormat->SetTypeLabel("0003");
                packFormat->SetTypeDefinition("Objects");
              }

              if (object == NULL)
              {
                // create object if it does not already exist
                object = CreateObject(objectname, content);
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
