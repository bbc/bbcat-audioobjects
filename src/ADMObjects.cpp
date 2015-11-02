
#include <stdio.h>
#include <math.h>

#include <map>
#include <algorithm>

#define DEBUG_LEVEL 1
#include "ADMObjects.h"
#include "ADMData.h"

BBC_AUDIOTOOLBOX_START

std::map<uint_t,std::string> ADMObject::typeLabelMap;
std::map<uint_t,std::string> ADMObject::formatLabelMap;

// absolute maximum time
const uint64_t ADMObject::MaxTime = (uint64_t)-1;

/*--------------------------------------------------------------------------------*/
/** Base constructor for all objects
 *
 * @param _owner an instance of ADMData that this object should belong to
 * @param _id unique ID for this object (specified as part of the ADM)
 * @param _name optional human-friendly name of the object
 *
 */
/*--------------------------------------------------------------------------------*/
ADMObject::ADMObject(ADMData& _owner, const std::string& _id, const std::string& _name) : owner(_owner),
                                                                                          id(_id),
                                                                                          name(_name),
                                                                                          typeLabel(TypeLabel_Unknown),
                                                                                          standarddef(false)
{
  if (typeLabelMap.size() == 0)
  {
    // populate typeLabel map
    SetTypeDefinition(TypeLabel_DirectSpeakers, "DirectSpeakers");
    SetTypeDefinition(TypeLabel_Matrix,         "Matrix");
    SetTypeDefinition(TypeLabel_Objects,        "Objects");
    SetTypeDefinition(TypeLabel_HOA,            "HOA");
    SetTypeDefinition(TypeLabel_Binaural,       "Binaural");
  }
}

/*--------------------------------------------------------------------------------*/
/** Set and Get object ID
 *
 * @param id new ID
 * @param start start index to find an ID from
 *
 * @note setting the ID updates the map held within the ADMData object
 * @note some types of ID start numbering at 0x1000, some at 0x0000
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetID(const std::string& _id, uint_t start)
{
  // get owner to change it and update its map of objects in the process
  owner.ChangeID(this, _id, start);
}

/*--------------------------------------------------------------------------------*/
/** Register this object with the owner
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::Register()
{
  owner.Register(this);
}

/*--------------------------------------------------------------------------------*/
/** Set object typeLabel
 *
 * @param type typeLabel index
 *
 * @note if type is a recognized typeLabel, typeDefinition will automatically be set!
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetTypeLabel(uint_t type)
{
  if (type != typeLabel)
  {
    DEBUG2(("%s(%s,%s): Change typeLabel from %04x to %04x", GetType().c_str(), GetName().c_str(), GetID().c_str(), typeLabel, type));

    typeLabel = type;

    // only update referenced objects if *not* in pure mode
    if (!owner.InPureMode())
    {
      // update typeDefinition if possible
      if (typeLabelMap.find(typeLabel) != typeLabelMap.end()) SetTypeDefinition(typeLabelMap[typeLabel]);

      // update typeLabel's of referenced objects
      UpdateRefTypeLabels();

      // in some cases, setting the typeLabel causes a change in ID
      UpdateID();
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Set and Get object typeDefinition
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetTypeDefinition(const std::string& str)
{
  if (str != typeDefinition)
  {
    DEBUG2(("%s(%s,%s): Change typeDefinition from '%s' to '%s'", GetType().c_str(), GetName().c_str(), GetID().c_str(), typeDefinition.c_str(), str.c_str()));

    typeDefinition = str;

    // only update referenced objects if *not* in pure mode
    if (!owner.InPureMode())
    {
      // update typeDefinitions's of referenced objects
      UpdateRefTypeDefinitions();
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Set typeLabel and typeDefinition (if valid and ADM is not in pure mode) in supplied object
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetTypeInfoInObject(ADMObject *obj) const
{
  // only update object if *not* in pure mode
  if (!owner.InPureMode())
  {
    if (!typeDefinition.empty())        obj->SetTypeDefinition(typeDefinition);
    if (typeLabel != TypeLabel_Unknown) obj->SetTypeLabel(typeLabel);
  }
}

/*--------------------------------------------------------------------------------*/
/** Update object's ID
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::UpdateID()
{
  // do nothing
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetValues()
{
  values.SetValue(typeLabel,      "typeLabel", true);
  values.SetValue(typeDefinition, "typeDefinition");
}

/*--------------------------------------------------------------------------------*/
/** Try to connect references after all objects have been set up
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetReferences()
{
  XMLValues::iterator it;

  // cycle through values looking for references to the specified object type
  for (it = values.begin(); (it != values.end());)
  {
    const ADMObject *obj  = NULL; // for neater response handling
    const XMLValue& value = *it;
    bool  refrejected = false;

    // the value name is reference name of object type
    if (value.name == ADMAudioContent::Reference)
    {
      ADMAudioContent *ref;

      // look up reference using owner ADMData object, try to cast it to an object of the correct type
      if ((ref = dynamic_cast<ADMAudioContent *>(owner.GetReference(value))) != NULL)
      {
        // save object for debugging purposes
        obj = ref;

        // store object reference
        refrejected = !Add(ref);
      }
    }
    else if (value.name == ADMAudioObject::Reference)
    {
      ADMAudioObject *ref;

      if ((ref = dynamic_cast<ADMAudioObject *>(owner.GetReference(value))) != NULL)
      {
        // save object for debugging purposes
        obj = ref;

        // store object reference
        refrejected = !Add(ref);
      }
    }
    else if (value.name == ADMAudioTrack::Reference)
    {
      ADMAudioTrack *ref;

      if ((ref = dynamic_cast<ADMAudioTrack *>(owner.GetReference(value))) != NULL)
      {
        // save object for debugging purposes
        obj = ref;

        // store object reference
        refrejected = !Add(ref);
      }
    }
    else if (value.name == ADMAudioPackFormat::Reference)
    {
      ADMAudioPackFormat *ref;

      if ((ref = dynamic_cast<ADMAudioPackFormat *>(owner.GetReference(value))) != NULL)
      {
        // save object for debugging purposes
        obj = ref;

        // store object reference
        refrejected = !Add(ref);
      }
    }
    else if (value.name == ADMAudioStreamFormat::Reference)
    {
      ADMAudioStreamFormat *ref;

      if ((ref = dynamic_cast<ADMAudioStreamFormat *>(owner.GetReference(value))) != NULL)
      {
        // save object for debugging purposes
        obj = ref;

        // store object reference
        refrejected = !Add(ref);
      }
    }
    else if (value.name == ADMAudioTrackFormat::Reference)
    {
      ADMAudioTrackFormat *ref;

      if ((ref = dynamic_cast<ADMAudioTrackFormat *>(owner.GetReference(value))) != NULL)
      {
        // save object for debugging purposes
        obj = ref;

        // store object reference
        refrejected = !Add(ref);
      }
    }
    else if (value.name == ADMAudioChannelFormat::Reference)
    {
      ADMAudioChannelFormat *ref;

      if ((ref = dynamic_cast<ADMAudioChannelFormat *>(owner.GetReference(value))) != NULL)
      {
        // save object for debugging purposes
        obj = ref;

        // store object reference
        refrejected = !Add(ref);
      }
    }
    else
    {
      ++it;
      // note continue to avoid removing non-reference values
      continue;
    }

    if (obj)
    {
      if (refrejected)
      {
        ERROR("Reference %s as reference '%s' for %s REJECTED",
              obj->ToString().c_str(),
              value.value.c_str(),
              ToString().c_str());
      }
      else
      {
        DEBUG3(("Found %s as reference '%s' for %s",
                obj->ToString().c_str(),
                value.value.c_str(),
                ToString().c_str()));
      }
    }
    else
    {
      ERROR("Cannot find %s reference '%s' for %s",
            value.name.c_str(), value.value.c_str(),
            ToString().c_str());
    }

    // REMOVE value (the reference) from the list
    it = values.erase(it);
  }
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  XMLValues::const_iterator it;
  XMLValue value;

  UNUSED_PARAMETER(objects);

  if (GetID() != "")
  {
    if (GetType() == ADMAudioTrack::Type) value.SetAttribute("UID",            GetID());
    else                                  value.SetAttribute(GetType() + "ID", GetID());
    objvalues.push_back(value);
  }

  if (GetName() != "")
  {
    value.SetAttribute(GetType() + "Name", GetName());
    objvalues.push_back(value);
  }

  // add existing list of values/attributes to list
  for (it = values.begin(); it != values.end(); ++it)
  {
    // only add non-empty attributes
    if (full || (it->value != "") || (it->attrs.end() != it->attrs.begin()))
    {
      objvalues.push_back(*it);
    }
  }

  // add values/attributes not held in 'values' to list
  if (full || (typeLabel != 0))
  {
    value.SetAttribute("typeLabel", typeLabel, "%04x");
    objvalues.push_back(value);
  }
  if (full || (typeDefinition != ""))
  {
    value.SetAttribute("typeDefinition", typeDefinition);
    objvalues.push_back(value);
  }
}

/*----------------------------------------------------------------------------------------------------*/

const std::string ADMAudioProgramme::Type      = "audioProgramme";
const std::string ADMAudioProgramme::Reference = Type + "IDRef";
const std::string ADMAudioProgramme::IDPrefix  = "APR_";

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioProgramme::SetValues()
{
  ADMObject::SetValues();

  values.SetValue(language, "language");
}

bool ADMAudioProgramme::Add(ADMAudioContent *obj)
{
  if (std::find(contentrefs.begin(), contentrefs.end(), obj) == contentrefs.end())
  {
    contentrefs.push_back(obj);
    return true;
  }

  // reference is already in the list
  return true;
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioProgramme::GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  XMLValue value;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // add values/attributes not held in 'values' to list
  if (full || (language != ""))
  {
    value.SetAttribute("language", language);
    objvalues.push_back(value);
  }

  // references only
  object.genref  = true;

  for (i = 0; i < contentrefs.size(); i++)
  {
    object.obj = contentrefs[i];
    objects.push_back(object);
  }
}

/*--------------------------------------------------------------------------------*/
/** Generate a textual list of references 
 *
 * @param str string to be modified
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioProgramme::GenerateReferenceList(std::string& str) const
{
  uint_t i;

  for (i = 0; i < contentrefs.size(); i++) GenerateReference(str, contentrefs[i]);
}

/*----------------------------------------------------------------------------------------------------*/

const std::string ADMAudioContent::Type      = "audioContent";
const std::string ADMAudioContent::Reference = Type + "IDRef";
const std::string ADMAudioContent::IDPrefix  = "ACO_";

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioContent::SetValues()
{
  ADMObject::SetValues();

  values.SetValue(language, "language");
}

bool ADMAudioContent::Add(ADMAudioObject *obj)
{
  if (std::find(objectrefs.begin(), objectrefs.end(), obj) == objectrefs.end())
  {
    objectrefs.push_back(obj);
    return true;
  }

  // reference is already in the list
  return true;
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioContent::GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  XMLValue value;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // add values/attributes not held in 'values' to list
  if (full || (language != ""))
  {
    value.SetAttribute("language", language);
    objvalues.push_back(value);
  }

  // references only
  object.genref  = true;

  for (i = 0; i < objectrefs.size(); i++)
  {
    object.obj = objectrefs[i];
    objects.push_back(object);
  }
}

/*--------------------------------------------------------------------------------*/
/** Generate a textual list of references 
 *
 * @param str string to be modified
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioContent::GenerateReferenceList(std::string& str) const
{
  uint_t i;

  for (i = 0; i < objectrefs.size(); i++) GenerateReference(str, objectrefs[i]);
}

/*----------------------------------------------------------------------------------------------------*/

const std::string ADMAudioObject::Type      = "audioObject";
const std::string ADMAudioObject::Reference = Type + "IDRef";
const std::string ADMAudioObject::IDPrefix  = "AO_";

/*--------------------------------------------------------------------------------*/
/** ADM AudioObject object
 *
 * @param _owner an instance of ADMData that this object should belong to
 * @param _id unique ID for this object (specified as part of the ADM)
 * @param _name optional human-friendly name of the object
 *
 * @note type passed to base constructor is fixed by static member variable Type 
 */
/*--------------------------------------------------------------------------------*/
ADMAudioObject::ADMAudioObject(ADMData& _owner, const std::string& _id, const std::string& _name) :
  ADMObject(_owner, _id, _name),
  AudioObject(),
  starttrack(0),
  startTime(0),
  duration(0),
  startTimeSet(false),
  durationSet(false)
{
  Register();
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioObject::SetValues()
{
  ADMObject::SetValues();

  uint64_t _time;
  if (values.SetValueTime(_time, "startTime")) SetStartTime(_time);
  if (values.SetValueTime(_time, "duration"))  SetDuration(_time);
}

bool ADMAudioObject::Add(ADMAudioObject *obj)
{
  if (std::find(objectrefs.begin(), objectrefs.end(), obj) == objectrefs.end())
  {
    objectrefs.push_back(obj);
    return true;
  }

  // reference is already in the list
  return true;
}

bool ADMAudioObject::Add(ADMAudioPackFormat *obj)
{
  if (std::find(packformatrefs.begin(), packformatrefs.end(), obj) == packformatrefs.end())
  {
    packformatrefs.push_back(obj);
    return true;
  }

  // reference is already in the list
  return true;
}

bool ADMAudioObject::Add(ADMAudioTrack *obj)
{
  if (std::find(trackrefs.begin(), trackrefs.end(), obj) == trackrefs.end())
  {
    trackrefs.push_back(obj);
    // add to track map
    trackmap[obj->GetTrackNum()] = obj;
    std::sort(trackrefs.begin(), trackrefs.end(), &ADMAudioTrack::Compare);
    starttrack = trackrefs[0]->GetTrackNum();
    return true;
  }

  // reference is already in the list
  return true;
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioObject::GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  XMLValue value;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // add values/attributes not held in 'values' to list
  if (full || StartTimeSet())
  {
    value.SetAttribute("startTime", startTime);
    objvalues.push_back(value);
  }
  if (full || DurationSet())
  {
    value.SetAttribute("duration",  duration);
    objvalues.push_back(value);
  }

  // references only
  object.genref  = true;

  for (i = 0; i < objectrefs.size(); i++)
  {
    object.obj = objectrefs[i];
    objects.push_back(object);
  }

  for (i = 0; i < packformatrefs.size(); i++)
  {
    object.obj = packformatrefs[i];
    objects.push_back(object);
  }

  for (i = 0; i < trackrefs.size(); i++)
  {
    object.obj = trackrefs[i];
    objects.push_back(object);
  }
}

/*--------------------------------------------------------------------------------*/
/** Generate a textual list of references 
 *
 * @param str string to be modified
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioObject::GenerateReferenceList(std::string& str) const
{
  uint_t i;

  for (i = 0; i < objectrefs.size(); i++) GenerateReference(str, objectrefs[i]);
  for (i = 0; i < packformatrefs.size(); i++) GenerateReference(str, packformatrefs[i]);
  for (i = 0; i < trackrefs.size(); i++) GenerateReference(str, trackrefs[i]);
}

/*--------------------------------------------------------------------------------*/
/** Get audioChannelFormat for a particular track
 */
/*--------------------------------------------------------------------------------*/
ADMAudioChannelFormat *ADMAudioObject::GetChannelFormat(uint_t track) const
{
  std::map<uint_t,const ADMAudioTrack *>::const_iterator it;
  ADMAudioChannelFormat *channelFormat = NULL;

  if ((it = trackmap.find(track)) != trackmap.end())
  {
    const ADMAudioTrack&                      audiotrack      = *it->second;
    const std::vector<ADMAudioTrackFormat *>& trackformatrefs = audiotrack.GetTrackFormatRefs();

    if (trackformatrefs.size() == 1)
    {
      const std::vector<ADMAudioStreamFormat *>& streamformatrefs = trackformatrefs[0]->GetStreamFormatRefs();

      if (streamformatrefs.size() == 1)
      {
        const std::vector<ADMAudioChannelFormat *>& channelformatrefs = streamformatrefs[0]->GetChannelFormatRefs();

        if (channelformatrefs.size() == 1)
        {
          channelFormat = channelformatrefs[0];
        }
        else ERROR("Incorrect channelformatrefs in '%s' (%u)!", streamformatrefs[0]->ToString().c_str(), (uint_t)channelformatrefs.size());
      }
      else ERROR("Incorrect streamformatrefs in '%s' (%u)!", trackformatrefs[0]->ToString().c_str(), (uint_t)streamformatrefs.size());
    }
    else ERROR("Incorrect trackformatrefs in '%s' (%u)!", audiotrack.ToString().c_str(), (uint_t)trackformatrefs.size());
  }

  return channelFormat;
}

/*--------------------------------------------------------------------------------*/
/** Get list of audioBlockFormats for a particular track
 */
/*--------------------------------------------------------------------------------*/
const std::vector<ADMAudioBlockFormat *> *ADMAudioObject::GetBlockFormatList(uint_t track) const
{
  const ADMAudioChannelFormat              *channelformat;
  const std::vector<ADMAudioBlockFormat *> *blockformats = NULL;

  if ((channelformat = GetChannelFormat(track)) != NULL)
  {
    blockformats = &channelformat->GetBlockFormatRefs();
  }

  return blockformats;
}

#if ENABLE_JSON
/*--------------------------------------------------------------------------------*/
/** Convert parameters to a JSON object
 *
 * ADM audio objects contain extra information for the JSON representation
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioObject::ToJSON(json_spirit::mObject& obj) const
{
  if (objectrefs.size())
  {
    json_spirit::mArray array;
    uint_t i;
    for (i = 0; i < objectrefs.size(); i++)
    {
      const ADMAudioObject& object = *objectrefs[i];
    
      array.push_back(object.GetID());
    }

    obj["objects"] = array;
  }
}
#endif

/*----------------------------------------------------------------------------------------------------*/

const std::string ADMAudioTrack::Type      = "audioTrackUID";
const std::string ADMAudioTrack::Reference = Type + "Ref";
const std::string ADMAudioTrack::IDPrefix  = "ATU_";

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrack::SetValues()
{
  ADMObject::SetValues();

  // trackNum is 1- based in XML, 0- based in object
  if (values.SetValue(trackNum, "trackNum")) trackNum--;
  values.SetValue(sampleRate, "sampleRate");
  values.SetValue(bitDepth,   "bitDepth");
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrack::GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  XMLValue value;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // add values/attributes not held in 'values' to list
  if (full)
  {
    value.SetAttribute("trackNum", trackNum + 1); objvalues.push_back(value);
  }
  value.SetAttribute("sampleRate", sampleRate); objvalues.push_back(value);
  value.SetAttribute("bitDepth",   bitDepth);   objvalues.push_back(value);

  // output references to objects
  object.genref = true;

  for (i = 0; i < trackformatrefs.size(); i++)
  {
    object.obj = trackformatrefs[i];
    objects.push_back(object);
  }

  for (i = 0; i < packformatrefs.size(); i++)
  {
    object.obj = packformatrefs[i];
    objects.push_back(object);
  }
}

bool ADMAudioTrack::Add(ADMAudioTrackFormat *obj)
{
  if (std::find(trackformatrefs.begin(), trackformatrefs.end(), obj) == trackformatrefs.end())
  {
    trackformatrefs.push_back(obj);
    return true;
  }

  // reference is already in the list
  return true;
}

bool ADMAudioTrack::Add(ADMAudioPackFormat *obj)
{
  if (std::find(packformatrefs.begin(), packformatrefs.end(), obj) == packformatrefs.end())
  {
    packformatrefs.push_back(obj);
    return true;
  }

  // reference is already in the list
  return true;
}

/*--------------------------------------------------------------------------------*/
/** Generate a textual list of references 
 *
 * @param str string to be modified
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrack::GenerateReferenceList(std::string& str) const
{
  uint_t i;

  for (i = 0; i < trackformatrefs.size(); i++) GenerateReference(str, trackformatrefs[i]);
  for (i = 0; i < packformatrefs.size(); i++) GenerateReference(str, packformatrefs[i]);
}

/*----------------------------------------------------------------------------------------------------*/

const std::string ADMAudioPackFormat::Type      = "audioPackFormat";
const std::string ADMAudioPackFormat::Reference = Type + "IDRef";
const std::string ADMAudioPackFormat::IDPrefix  = "AP_";

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioPackFormat::SetValues()
{
  ADMObject::SetValues();
}

/*--------------------------------------------------------------------------------*/
/** Update object's ID
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioPackFormat::UpdateID()
{
  // call SetID() with new ID
  std::string _id;

  Printf(_id, "%04x%%04x", typeLabel);

  // custom pack formats start indexing at 0x1000
  SetID(GetIDPrefix() + _id, 0x1000);
}

/*--------------------------------------------------------------------------------*/
/** Add reference to an AudioChannelFormat object
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioPackFormat::Add(ADMAudioChannelFormat *obj)
{
  if (std::find(channelformatrefs.begin(), channelformatrefs.end(), obj) == channelformatrefs.end())
  {
    SetTypeInfoInObject(obj);
    channelformatrefs.push_back(obj);
    return true;
  }

  // reference is already in the list
  return true;
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioPackFormat::GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  XMLValue         value;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // generate references only
  object.genref  = true;

  for (i = 0; i < channelformatrefs.size(); i++)
  {
    object.obj = channelformatrefs[i];
    objects.push_back(object);
  }

  for (i = 0; i < packformatrefs.size(); i++)
  {
    object.obj = packformatrefs[i];
    objects.push_back(object);
  }
}

bool ADMAudioPackFormat::Add(ADMAudioPackFormat *obj)
{
  if (std::find(packformatrefs.begin(), packformatrefs.end(), obj) == packformatrefs.end())
  {
    SetTypeInfoInObject(obj);
    packformatrefs.push_back(obj);
    return true;
  }

  // reference is already in the list
  return true;
}

/*--------------------------------------------------------------------------------*/
/** Generate a textual list of references 
 *
 * @param str string to be modified
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioPackFormat::GenerateReferenceList(std::string& str) const
{
  uint_t i;

  for (i = 0; i < channelformatrefs.size(); i++) GenerateReference(str, channelformatrefs[i]);
  for (i = 0; i < packformatrefs.size(); i++) GenerateReference(str, packformatrefs[i]);
}

/*----------------------------------------------------------------------------------------------------*/

const std::string ADMAudioStreamFormat::Type      = "audioStreamFormat";
const std::string ADMAudioStreamFormat::Reference = Type + "IDRef";
const std::string ADMAudioStreamFormat::IDPrefix  = "AS_";

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::SetValues()
{
  ADMObject::SetValues();

  values.SetValue(formatLabel,      "formatLabel", true);
  values.SetValue(formatDefinition, "formatDefinition");
}

/*--------------------------------------------------------------------------------*/
/** Update object's ID
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::UpdateID()
{
  // call SetID() with new ID
  std::string _id;
  uint_t i;

  Printf(_id, "%04x%%04x", typeLabel);

  // custom stream formats start indexing at 0x1000
  SetID(GetIDPrefix() + _id, 0x1000);

  // set referenced trackformats' IDs

  // form ID using streamformat's ID with track number suffixed
  _id = "";
  Printf(_id, "%s_%%02x", GetID().substr(GetIDPrefix().length()).c_str());
  for (i = 0; i < trackformatrefs.size(); i++)
  {
    trackformatrefs[i]->SetID(trackformatrefs[i]->GetIDPrefix() + _id);
  }
}

/*--------------------------------------------------------------------------------*/
/** Add reference to an AudioChannelFormat object
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioStreamFormat::Add(ADMAudioChannelFormat *obj)
{
  if (channelformatrefs.size() == 0)
  {
    SetTypeInfoInObject(obj);
    channelformatrefs.push_back(obj);
    return true;
  }

  // only a single reference allowed -> overwrite existing
  channelformatrefs[0] = obj;
  return true;
}

/*--------------------------------------------------------------------------------*/
/** Add reference to an AudioTrackFormat object
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioStreamFormat::Add(ADMAudioTrackFormat *obj)
{
  if (std::find(trackformatrefs.begin(), trackformatrefs.end(), obj) == trackformatrefs.end())
  {
    SetTypeInfoInObject(obj);
    trackformatrefs.push_back(obj);
    return true;
  }

  // reference is already in the list
  return true;
}

/*--------------------------------------------------------------------------------*/
/** Add reference to an AudioPackFormat object
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioStreamFormat::Add(ADMAudioPackFormat *obj)
{
  if (packformatrefs.size() == 0)
  {
    SetTypeInfoInObject(obj);
    packformatrefs.push_back(obj);
    return true;
  }

  // only a single reference allowed -> overwrite existing
  packformatrefs[0] = obj;
  return true;
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  XMLValue value;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // add values/attributes not held in 'values' to list
  if (full || (formatLabel != 0))
  {
    value.SetAttribute("formatLabel", formatLabel, "%04x");
    objvalues.push_back(value);
  }
  if (full || (formatDefinition != ""))
  {
    value.SetAttribute("formatDefinition", formatDefinition);
    objvalues.push_back(value);
  }

  // generate references only
  object.genref  = true;

  for (i = 0; i < channelformatrefs.size(); i++)
  {
    object.obj = channelformatrefs[i];
    objects.push_back(object);
  }

  for (i = 0; i < trackformatrefs.size(); i++)
  {
    object.obj = trackformatrefs[i];
    objects.push_back(object);
  }

  for (i = 0; i < packformatrefs.size(); i++)
  {
    object.obj = packformatrefs[i];
    objects.push_back(object);
  }
}

/*--------------------------------------------------------------------------------*/
/** Generate a textual list of references 
 *
 * @param str string to be modified
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::GenerateReferenceList(std::string& str) const
{
  uint_t i;

  for (i = 0; i < channelformatrefs.size(); i++) GenerateReference(str, channelformatrefs[i]);
  for (i = 0; i < packformatrefs.size(); i++) GenerateReference(str, packformatrefs[i]);
  for (i = 0; i < trackformatrefs.size(); i++) GenerateReference(str, trackformatrefs[i]);
}

/*----------------------------------------------------------------------------------------------------*/

const std::string ADMAudioTrackFormat::Type      = "audioTrackFormat";
const std::string ADMAudioTrackFormat::Reference = Type + "IDRef";
const std::string ADMAudioTrackFormat::IDPrefix  = "AT_";

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrackFormat::SetValues()
{
  ADMObject::SetValues();

  values.SetValue(formatLabel,      "formatLabel", true);
  values.SetValue(formatDefinition, "formatDefinition");
}

/*--------------------------------------------------------------------------------*/
/** Update object's ID
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrackFormat::UpdateID()
{
  // ONLY update this trackformat's ID if there are no streamformat references
  // (otherwise the streamformat will sort this trackformat out)

  if (!streamformatrefs.size())
  {
    // call SetID() with new ID
    std::string _id;

    Printf(_id, "%04x1000_%%02x", typeLabel);

    // custom track formats start indexing at 0x1000
    SetID(GetIDPrefix() + _id);
  }
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrackFormat::GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  XMLValue value;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // add values/attributes not held in 'values' to list
  if (full || (formatLabel != 0))
  {
    value.SetAttribute("formatLabel", formatLabel, "%04x");
    objvalues.push_back(value);
  }
  if (full || (formatDefinition != ""))
  {
    value.SetAttribute("formatDefinition", formatDefinition);
    objvalues.push_back(value);
  }

  // generate references only
  object.genref  = true;

  for (i = 0; i < streamformatrefs.size(); i++)
  {
    object.obj = streamformatrefs[i];
    objects.push_back(object);
  }
}

bool ADMAudioTrackFormat::Add(ADMAudioStreamFormat *obj)
{
  if (streamformatrefs.size() == 0)
  {
    SetTypeInfoInObject(obj);
    streamformatrefs.push_back(obj);
    return true;
  }

  // only a single reference allowed -> overwrite existing
  streamformatrefs[0] = obj;
  return true;
}

/*--------------------------------------------------------------------------------*/
/** Generate a textual list of references 
 *
 * @param str string to be modified
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrackFormat::GenerateReferenceList(std::string& str) const
{
  uint_t i;

  for (i = 0; i < streamformatrefs.size(); i++) GenerateReference(str, streamformatrefs[i]);
}

/*----------------------------------------------------------------------------------------------------*/

const std::string ADMAudioChannelFormat::Type      = "audioChannelFormat";
const std::string ADMAudioChannelFormat::Reference = Type + "IDRef";
const std::string ADMAudioChannelFormat::IDPrefix  = "AC_";

ADMAudioChannelFormat::~ADMAudioChannelFormat()
{
  // delete all block formats
  uint_t i;
  for (i = 0; i < blockformatrefs.size(); i++) delete blockformatrefs[i];
  blockformatrefs.clear();
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioChannelFormat::SetValues()
{
  ADMObject::SetValues();
}

/*--------------------------------------------------------------------------------*/
/** Update object's ID
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioChannelFormat::UpdateID()
{
  // call SetID() with new ID
  std::string _id;

  Printf(_id, "%04x%%04x", typeLabel);

  // custom channel formats start indexing at 0x1000
  SetID(GetIDPrefix() + _id, 0x1000);
}

/*--------------------------------------------------------------------------------*/
/** Sort block formats in time order
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioChannelFormat::SortBlockFormats()
{
  std::sort(blockformatrefs.begin(), blockformatrefs.end(), ADMAudioBlockFormat::Compare);
}

/*--------------------------------------------------------------------------------*/
/** Add reference to an AudioBlockFormat object and ensures blocks are sorted by time
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioChannelFormat::Add(ADMAudioBlockFormat *obj)
{
  uint_t n = (uint_t)blockformatrefs.size();
  uint64_t t = obj->GetStartTime();

  // insert obj into list ordered by time

  // allocate memory in large chunks to reduce re-allocation time
  if (n == blockformatrefs.capacity()) blockformatrefs.reserve(blockformatrefs.capacity() + 1024);

  // if the list is empty just append
  if (!n)
  {
    DEBUG3(("blockformat list is empty: new item appended"));
    blockformatrefs.push_back(obj);
  }
  // the most likely place for the new item is on the end so check this first
  else if (t >= blockformatrefs[n - 1]->GetStartTime())
  {
    // ensure the new object is not already in the list
    if (obj != blockformatrefs[n - 1])
    {
      // new object just needs to be appended to list
      DEBUG3(("New item is beyond last item (%llu >= %llu): new item appended", t, blockformatrefs[n - 1]->GetStartTime()));
      blockformatrefs.push_back(obj);
    }
  }
  // if the list has only one item or the new object is before the first item, new item must be inserted at the start
  else if ((n == 1) || (t <= blockformatrefs[0]->GetStartTime()))
  {
    // ensure the new object is not already in the list
    if (obj != blockformatrefs[0])
    {
      // new object just needs inserted at the start of the list
      DEBUG3(("New item is before first item (%llu <= %llu): new item inserted at start", t, blockformatrefs[0]->GetStartTime()));
      blockformatrefs.insert(blockformatrefs.begin(), obj);
    }
  }
  // object should be placed somewhere in the list but not at the end (checked above)
  // list has at least two entries
  else
  {
    // start at the middle of the list and use a binary search
    uint_t inc = (n + 1) >> 1;      // round up division
    uint_t pos = MIN(inc, n - 2);                                 // start in the middle but ensure the last position is never checked (no point)
    uint_t count = 0;

    // break out when the new object is between the current and next item
    while (!((t >= blockformatrefs[pos]->GetStartTime()) &&
             (t <  blockformatrefs[pos + 1]->GetStartTime())))
    {
      // half increment (rounding up to ensure it is always non-zero)
      inc = (inc + 1) >> 1;

      // if the new item is before the current one, move back by the increment
      if (t < blockformatrefs[pos]->GetStartTime())
      {
        if (pos >= inc) pos -= inc;
        else            pos  = 0;
      }
      // else move forward by the increment, limiting to the end but one item
      else pos = MIN(pos + inc, n - 2);

      count++;
    }

    // check that the item isn't already in the list
    if (obj != blockformatrefs[pos])
    {
      (void)count;

      DEBUG3(("New item is between indexes %u and %u (%llu <= %llu < %llu) (%u iterations): new item inserted between them", pos, pos + 1, blockformatrefs[pos]->GetStartTime(), t, blockformatrefs[pos + 1]->GetStartTime(), count));

      // the new item is between the pos'th and pos+1'th item
      // but insert works by inserting *before* the given position
      // therefore the position needs to be incremented by one
      pos++;

      blockformatrefs.insert(blockformatrefs.begin() + pos, obj);
    }
  }

  return true;
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioChannelFormat::GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);
}

/*--------------------------------------------------------------------------------*/
/** Generate a textual list of references 
 *
 * @param str string to be modified
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioChannelFormat::GenerateReferenceList(std::string& str) const
{
  UNUSED_PARAMETER(str);
}

/*--------------------------------------------------------------------------------*/
/** Provide a way of accessing contained items without knowing what they are
 * (used for block formats)
 *
 * @param n object index
 * @param object structure to be filled
 *
 * @return true if object valid
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioChannelFormat::GetContainedObject(uint_t n, CONTAINEDOBJECT& object) const
{
  bool success = false;

  if (n < blockformatrefs.size())
  {
    const ADMAudioBlockFormat *block = blockformatrefs[n];
    
    object.type = block->GetType();
    object.attrs.clear();
    object.values.clear();

    XMLValue value;
    std::string id;
    Printf(id, "%s%s_%08x", block->GetIDPrefix().c_str(), GetID().substr(GetIDPrefix().length()).c_str(), n + 1);
    value.SetAttribute(block->GetType() + "ID", id);
    object.attrs.AddValue(value);
    
    block->GetValues(object.attrs, object.values);
    
    success = true;
  }
  
  return success;
}

/*----------------------------------------------------------------------------------------------------*/

const std::string ADMAudioBlockFormat::Type      = "audioBlockFormat";
const std::string ADMAudioBlockFormat::Reference = Type + "IDRef";
const std::string ADMAudioBlockFormat::IDPrefix  = "AB_";

/*--------------------------------------------------------------------------------*/
/** ADM AudioBlockFormat object
 */
/*--------------------------------------------------------------------------------*/
ADMAudioBlockFormat::ADMAudioBlockFormat() :
  rtime(0),
  duration(0),
  rtimeSet(false),
  durationSet(false)
{
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioBlockFormat::SetValues(XMLValues& values)
{
  XMLValues::iterator it;
  ParameterSet othervalues;
  Position     position;

  uint64_t _time;
  if (values.SetValueTime(_time, "rtime"))    SetStartTime(_time);
  if (values.SetValueTime(_time, "duration")) SetDuration(_time);

  for (it = values.begin(); it != values.end();)
  {
    const XMLValue& value = *it;

    if (value.name == "position")
    {
      double val;

      if (sscanf(value.value.c_str(), "%lf", &val) > 0)
      {
        const std::string *attr;

        if ((attr = value.GetAttribute("coordinate")) != NULL)
        {
          DEBUG4(("Position type %s value %0.6lf", attr->c_str(), val));

          if      (*attr == "azimuth")   {position.pos.az = val; position.polar = true;}
          else if (*attr == "elevation") {position.pos.el = val; position.polar = true;}
          else if (*attr == "distance")  {position.pos.d  = val; position.polar = true;}
          else if (*attr == "x")         {position.pos.x  = val; position.polar = false;}
          else if (*attr == "y")         {position.pos.y  = val; position.polar = false;}
          else if (*attr == "z")         {position.pos.z  = val; position.polar = false;}
        }
      }
      else ERROR("Failed to evaluate '%s' as floating point number for position", value.value.c_str());

      it = values.erase(it);
    }
    else if (value.name == "Dialogue")
    {
      uint_t val;

      if (Evaluate(value.value, val)) objparameters.SetDialogue(val);

      it = values.erase(it);
    }
    else if (value.name == "Interact")
    {
      bool val;

      if (Evaluate(value.value, val)) objparameters.SetInteract(val);

      it = values.erase(it);
    }
    else if (value.name == "Importance")
    {
      int val;

      if (Evaluate(value.value, val)) objparameters.SetImportance(val);

      it = values.erase(it);
    }
    else if (value.name == "width")
    {
      double val;

      if (Evaluate(value.value, val)) objparameters.SetWidth(val);

      it = values.erase(it);
    }
    else if (value.name == "depth")
    {
      double val;

      if (Evaluate(value.value, val)) objparameters.SetDepth(val);

      it = values.erase(it);
    }
    else if (value.name == "height")
    {
      double val;

      if (Evaluate(value.value, val)) objparameters.SetHeight(val);

      it = values.erase(it);
    }
    else if (value.name == "gain")
    {
      double val;

      if (Evaluate(value.value, val)) objparameters.SetGain(val);

      it = values.erase(it);
    }
    else if (value.name == "diffuse")
    {
      double val;

      if (Evaluate(value.value, val)) objparameters.SetDiffuseness(val);

      it = values.erase(it);
    }
    else if (value.name == "jumpPosition")
    {
      bool val;

      if (Evaluate(value.value, val)) objparameters.SetInterpolate(!val);

      { // read interpolationTime, if it exists
        XMLValue::ATTRS::const_iterator it2;
        double fval;
        
        if (((it2 = value.attrs.find("interpolationTime")) != value.attrs.end()) && Evaluate(it2->second, fval)) objparameters.SetInterpolationTimeS(fval);
      }
      
      it = values.erase(it);
    }
    else if (value.name == "channelLock")
    {
      bool val;

      if (Evaluate(value.value, val)) objparameters.SetChannelLock(val);

      it = values.erase(it);
    }
    else // any other parameters -> assume they are part of the supplement information
    {
      othervalues.Set(value.name, value.value);

      it = values.erase(it);
    }
  }

  // set values within the audio object parameters object
  objparameters.SetPosition(position);
  objparameters.SetOtherValues(othervalues);
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 *
 * @param objattrs list to be populated with XMLValue's holding object attributes
 * @param objvalues list to be populated with XMLValue's holding object values
 * @param full true to generate complete list including values that do not appear in the XML
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioBlockFormat::GetValues(XMLValues& objattrs, XMLValues& objvalues, bool full) const
{
  XMLValue value;
  double   fval;
  int      ival;
  bool     bval;

  // add values/attributes not held in 'values' to list
  if (full || RTimeSet())
  {
    value.SetAttribute("rtime", rtime);
    objattrs.push_back(value);
  }
  if (full || DurationSet())
  {
    value.SetAttribute("duration", duration);
    objattrs.push_back(value);
  }

  const Position& position = objparameters.GetPosition();
  if (position.polar)
  {
    value.SetValue("position", position.pos.az);
    value.SetValueAttribute("coordinate", "azimuth");
    objvalues.push_back(value);

    value.SetValue("position", position.pos.el);
    value.SetValueAttribute("coordinate", "elevation");
    objvalues.push_back(value);

    value.SetValue("position", position.pos.d);
    value.SetValueAttribute("coordinate", "distance");
    objvalues.push_back(value);
  }
  else
  {
    value.SetValue("position", position.pos.x, "%0.10lf");
    value.SetValueAttribute("coordinate", "x");
    objvalues.push_back(value);

    value.SetValue("position", position.pos.y, "%0.10lf");
    value.SetValueAttribute("coordinate", "y");
    objvalues.push_back(value);

    value.SetValue("position", position.pos.z, "%0.10lf");
    value.SetValueAttribute("coordinate", "z");
    objvalues.push_back(value);
  }

  if (objparameters.GetGain(fval))
  {
    value.SetValue("gain", fval, "%0.10lf");
    objvalues.push_back(value);
  }

  if (objparameters.GetWidth(fval))
  {
    value.SetValue("width", fval, "%0.10lf");
    objvalues.push_back(value);
  }

  if (objparameters.GetDepth(fval))
  {
    value.SetValue("depth", fval, "%0.10lf");
    objvalues.push_back(value);
  }

  if (objparameters.GetHeight(fval))
  {
    value.SetValue("height", fval, "%0.10lf");
    objvalues.push_back(value);
  }

  if (objparameters.GetDiffuseness(fval))
  {
    value.SetValue("diffuse", fval);
    objvalues.push_back(value);
  }

  if (objparameters.GetInterpolate(bval))
  {
    double itime;
    
    value.SetValue("jumpPosition", !bval);    // NOTE: inverted!

    // set interpolation time
    if (objparameters.GetInterpolationTimeS(itime)) value.SetValueAttribute("interpolationTime", itime);
    
    objvalues.push_back(value);
  }

  if (objparameters.GetInteract(bval))
  {
    value.SetValue("Interact", bval);
    objvalues.push_back(value);
  }

  if (objparameters.GetChannelLock(bval))
  {
    value.SetValue("channelLock", bval);
    objvalues.push_back(value);
  }

  if (objparameters.GetDialogue(ival))
  {
    value.SetValue("Dialogue", ival);
    objvalues.push_back(value);
  }

  if (objparameters.GetImportance(ival))
  {
    value.SetValue("Importance", ival);
    objvalues.push_back(value);
  }

  // add all parameters from the supplement information
  const ParameterSet& othervalues = objparameters.GetOtherValues();
  ParameterSet::Iterator it;
  for (it = othervalues.GetBegin(); it != othervalues.GetEnd(); ++it)
  {
    value.SetValue(it->first, it->second);
    objvalues.push_back(value);
  }
}

/*--------------------------------------------------------------------------------*/
/** Generate a textual list of references 
 *
 * @param str string to be modified
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioBlockFormat::GenerateReferenceList(std::string& str) const
{
  UNUSED_PARAMETER(str);
}

#if ENABLE_JSON
/*--------------------------------------------------------------------------------*/
/** Convert parameters to a JSON object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioBlockFormat::ToJSON(json_spirit::mObject& obj) const
{
  obj["startTime"]  = (boost::uint64_t)GetStartTime();
  obj["duration"]   = (boost::uint64_t)GetDuration();
  obj["parameters"] = objparameters.ToJSON();
}
#endif

/*----------------------------------------------------------------------------------------------------*/

ADMTrackCursor::ADMTrackCursor(uint_t _channel) : AudioObjectCursor(),
                                                  channel(_channel),
                                                  objectindex(0),
                                                  blockindex(0),
                                                  currenttime(0),
                                                  blockformatstarted(false),
                                                  objparametersvalid(false)
{
}

ADMTrackCursor::ADMTrackCursor(const ADMTrackCursor& obj) : AudioObjectCursor(),
                                                            channel(obj.channel),
                                                            objectindex(0),
                                                            blockindex(0),
                                                            currenttime(0),
                                                            blockformatstarted(false),
                                                            objparametersvalid(false)
{
  uint_t i;

  for (i = 0; i < obj.objectlist.size(); i++) Add(obj.objectlist[i].object, false);

  Sort();
}

ADMTrackCursor::~ADMTrackCursor()
{
}

/*--------------------------------------------------------------------------------*/
/** Add audio object to this object
 *
 * @return true if object added, false if object ignored
 */
/*--------------------------------------------------------------------------------*/
bool ADMTrackCursor::Add(const ADMAudioObject *object, bool sort)
{
  ThreadLock lock(tlock);
  uint_t i;
  bool   added = false;

  // look for this object in the list
  for (i = 0; i < objectlist.size(); i++)
  {
    // if object is already in list, exit now
    if (objectlist[i].object == object) return false;
  }

  AUDIOOBJECT obj =
  {
    object,
    object->GetChannelFormat(channel),
  };
  if (obj.channelformat)
  {
    objectlist.push_back(obj);

    DEBUG3(("Cursor<%016lx:%u>: Added object '%s', %u blocks", (ulong_t)this, channel, object->ToString().c_str(), (uint_t)obj.channelformat->GetBlockFormatRefs().size()));

    added = true;
  }

  if (added && sort) Sort();

  return added;
}

/*--------------------------------------------------------------------------------*/
/** Add audio objects to this object
 */
/*--------------------------------------------------------------------------------*/
bool ADMTrackCursor::Add(const ADMAudioObject *objects[], uint_t n)
{
  ThreadLock lock(tlock);
  bool   added = false;
  uint_t i;

  for (i = 0; i < n; i++) added |= Add(objects[i], false);

  if (added) Sort();

  return added;
}

/*--------------------------------------------------------------------------------*/
/** Add audio objects to this object
 */
/*--------------------------------------------------------------------------------*/
bool ADMTrackCursor::Add(const std::vector<const ADMAudioObject *>& objects)
{
  ThreadLock lock(tlock);
  bool   added = false;
  uint_t i;

  for (i = 0; i < objects.size(); i++) added |= Add(objects[i], false);

  if (added) Sort();

  return added;
}

/*--------------------------------------------------------------------------------*/
/** Add audio objects to this object
 */
/*--------------------------------------------------------------------------------*/
bool ADMTrackCursor::Add(const std::vector<const ADMObject *>& objects)
{
  ThreadLock lock(tlock);
  bool   added = false;
  uint_t i;

  for (i = 0; i < objects.size(); i++)
  {
    const ADMAudioObject *obj;

    if ((obj = dynamic_cast<const ADMAudioObject *>(objects[i])) != NULL)
    {
      added |= Add(obj, false);
    }
  }

  if (added) Sort();

  return added;
}

/*--------------------------------------------------------------------------------*/
/** Sort list of objects into time order
 */
/*--------------------------------------------------------------------------------*/
void ADMTrackCursor::Sort()
{
  ThreadLock lock(tlock);

  std::sort(objectlist.begin(), objectlist.end(), &Compare);

  Seek(currenttime);
}

/*--------------------------------------------------------------------------------*/
/** Return cursor start time in ns
 */
/*--------------------------------------------------------------------------------*/
uint64_t ADMTrackCursor::GetStartTime() const
{
  ThreadLock lock(tlock);
  uint64_t t = 0;

  if (objectlist.size() > 0)
  {
    const AUDIOOBJECT&                        object       = objectlist[0];
    const std::vector<ADMAudioBlockFormat *>& blockformats = object.channelformat->GetBlockFormatRefs();

    if (blockformats.size() > 0)
    {
      DEBUG3(("Object %u/%u start %lu BlockFormat %u/%u start %lu",
              0, (uint_t)objectlist.size(),   (ulong_t)object.object->GetStartTime(),
              0, (uint_t)blockformats.size(), (ulong_t)blockformats[0]->GetStartTime()));
            
      t = blockformats[0]->GetStartTime(object.object);
    }
  }

  return t;
}

/*--------------------------------------------------------------------------------*/
/** Return cursor end time in ns
 */
/*--------------------------------------------------------------------------------*/
uint64_t ADMTrackCursor::GetEndTime() const
{
  ThreadLock lock(tlock);
  uint64_t t = 0;

  if (objectlist.size() > 0)
  {
    const AUDIOOBJECT&                        object       = objectlist[objectlist.size() - 1];
    const std::vector<ADMAudioBlockFormat *>& blockformats = object.channelformat->GetBlockFormatRefs();

    if (blockformats.size() > 0)
    {
      DEBUG3(("Object %u/%u start %lu BlockFormat %u/%u start %lu duration %lu",
              (uint_t)(objectlist.size() - 1),   (uint_t)objectlist.size(),   (ulong_t)object.object->GetStartTime(),
              (uint_t)(blockformats.size() - 1), (uint_t)blockformats.size(), (ulong_t)blockformats[0]->GetStartTime(), (ulong_t)blockformats[0]->GetDuration()));

      t = blockformats[blockformats.size() - 1]->GetEndTime(object.object);
    }
  }

  return t;
}

/*--------------------------------------------------------------------------------*/
/** Return audio object parameters at current time
 */
/*--------------------------------------------------------------------------------*/
const ADMAudioBlockFormat *ADMTrackCursor::GetBlockFormat() const
{
  ThreadLock lock(tlock);
  const ADMAudioBlockFormat *blockformat = NULL;

  if (objectindex < objectlist.size())
  {
    const AUDIOOBJECT&                        object       = objectlist[objectindex];
    const std::vector<ADMAudioBlockFormat *>& blockformats = object.channelformat->GetBlockFormatRefs();
    
    blockformat = blockformats[blockindex];
  }
  
  return blockformat;
}

/*--------------------------------------------------------------------------------*/
/** Get current audio object
 */
/*--------------------------------------------------------------------------------*/
AudioObject *ADMTrackCursor::GetAudioObject() const
{
  ThreadLock lock(tlock);
  AudioObject *obj = NULL;

  if ((objectindex < objectlist.size()) && RANGE(currenttime, objectlist[objectindex].object->GetStartTime(), objectlist[objectindex].object->GetEndTime()))
  {
    obj = const_cast<ADMAudioObject *>(objectlist[objectindex].object);
  }

  return obj;
}

/*--------------------------------------------------------------------------------*/
/** Return audio object parameters at current time
 */
/*--------------------------------------------------------------------------------*/
bool ADMTrackCursor::GetObjectParameters(AudioObjectParameters& currentparameters) const
{
  ThreadLock lock(tlock);

  // only set parameters if they are valid
  if (objparametersvalid)
  {
    currentparameters = objparameters;
    return true;
  }

  return false;
}

/*--------------------------------------------------------------------------------*/
/** Start a blockformat at t
 */
/*--------------------------------------------------------------------------------*/
ADMAudioBlockFormat *ADMTrackCursor::StartBlockFormat(uint64_t t)
{
  ThreadLock lock(tlock);
  AUDIOOBJECT&        object       = objectlist[objectindex];
  ADMAudioBlockFormat *blockformat;

  if ((blockformat = new ADMAudioBlockFormat) != NULL)
  {
    blockformat->SetStartTime(t, object.object);
    object.channelformat->Add(blockformat);
    
    blockindex         = (uint_t)object.channelformat->GetBlockFormatRefs().size() - 1;
    blockformatstarted = true;

    DEBUG3(("Cursor<%016lx:%u>: Created new blockformat %u at %0.3lfs for object '%s', channelformat '%s'", (ulong_t)this, channel, blockindex, (double)t * 1.0e-9, object.object->ToString().c_str(), object.channelformat->ToString().c_str()));
  }

  return blockformat;
}

/*--------------------------------------------------------------------------------*/
/** End a blockformat at t that has previously been start
 */
/*--------------------------------------------------------------------------------*/
void ADMTrackCursor::EndBlockFormat(uint64_t t)
{
  ThreadLock lock(tlock);
  if (blockformatstarted)
  {
    AUDIOOBJECT&        object       = objectlist[objectindex];
    ADMAudioBlockFormat *blockformat = object.channelformat->GetBlockFormatRefs()[blockindex];
  
    blockformat->SetEndTime(t, object.object);

    DEBUG3(("Cursor<%016lx:%u>: Completed blockformat %u at %0.3lfs (duration %0.3lfs) for object '%s', channelformat '%s'", (ulong_t)this, channel, blockindex, (double)t * 1.0e-9, (double)blockformat->GetDuration() * 1.0e-9, object.object->ToString().c_str(), object.channelformat->ToString().c_str()));

    blockformatstarted = false;
  }
}

/*--------------------------------------------------------------------------------*/
/** Set audio object parameters for current time
 */
/*--------------------------------------------------------------------------------*/
void ADMTrackCursor::SetObjectParameters(const AudioObjectParameters& newparameters)
{
  ThreadLock lock(tlock);

  if ((objectindex < objectlist.size()) && (currenttime >= objectlist[objectindex].object->GetStartTime()))
  {
    AUDIOOBJECT&                        object       = objectlist[objectindex];
    std::vector<ADMAudioBlockFormat *>& blockformats = object.channelformat->GetBlockFormatRefs();

    // decide whether a new blockformat is required
    if (!objparametersvalid              || // if no existing entries; or
        (newparameters != objparameters))   // if parameters have changed
    {
      ADMAudioBlockFormat *blockformat;
      uint64_t            relativetime = currenttime - object.object->GetStartTime();

      // update internal parameters
      objparameters      = newparameters;
      objparametersvalid = true;

      if ((blockindex < blockformats.size()) && (blockformats[blockindex]->GetStartTime() == relativetime))
      {
        // new position at same time as original -> just update the parameters
        blockformats[blockindex]->GetObjectParameters() = objparameters;
        DEBUG2(("Updating channel %u to {'%s'}", channel, blockformats[blockindex]->GetObjectParameters().ToString().c_str()));
      }
      else
      {
        // new position requires new block format
        EndBlockFormat(currenttime);

        if ((blockformat = StartBlockFormat(currenttime)) != NULL)
        {
          blockformat->GetObjectParameters() = objparameters;
          DEBUG2(("Updating channel %u to {'%s'}", channel, blockformats[blockindex]->GetObjectParameters().ToString().c_str()));
        }
      }
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** End position updates by marking the end of the last block
 */
/*--------------------------------------------------------------------------------*/
void ADMTrackCursor::EndChanges()
{
  ThreadLock lock(tlock);

  if ((objectindex < objectlist.size()) &&
      (currenttime >= objectlist[objectindex].object->GetStartTime()) &&
      (objectlist[objectindex].channelformat->GetBlockFormatRefs().size() == 0))
  {
    // no blockformats for the current object, create one from start of object
    StartBlockFormat(objectlist[objectindex].object->GetStartTime());
  }

  // close last blockformat off by setting end time
  EndBlockFormat(currenttime);
}

/*--------------------------------------------------------------------------------*/
/** Get position at specified time (ns)
 */
/*--------------------------------------------------------------------------------*/
bool ADMTrackCursor::Seek(uint64_t t)
{
  ThreadLock lock(tlock);

  uint_t oldobjectindex = objectindex;
  uint_t oldblockindex  = blockindex;

  if (objectindex < objectlist.size())
  {
    // find right object in list
    while ((objectindex > 0) && (t < objectlist[objectindex].object->GetStartTime()))
    {
      // close last blockformat of this object off by setting end time
      EndBlockFormat(currenttime);

      // move back
      objectindex--;

      // reset blockindex
      blockindex = 0;

      // if the blockformat list is not empty, set the bockindex to the last one
      const std::vector<ADMAudioBlockFormat *>& blockformats = objectlist[objectindex].channelformat->GetBlockFormatRefs();
      if (blockformats.size() > 0) blockindex = (uint_t)blockformats.size() - 1;
    }
    while (((objectindex + 1) < objectlist.size()) && (t >= objectlist[objectindex + 1].object->GetStartTime()))
    {
      // close last blockformat of this object off by setting end time at start of next object
      EndBlockFormat(objectlist[objectindex + 1].object->GetStartTime());

      // move forward
      objectindex++;
      blockindex = 0;
    }

    // move blockindex as needed
    const AUDIOOBJECT& object = objectlist[objectindex];
    const std::vector<ADMAudioBlockFormat *>& blockformats = object.channelformat->GetBlockFormatRefs();
    if ((t >= object.object->GetStartTime()) && (blockindex < blockformats.size()))
    {
      // find right blockformat within object
      while ((blockindex       > 0)                   && (t <  blockformats[blockindex]->GetStartTime(object.object)))     blockindex--;
      while (((blockindex + 1) < blockformats.size()) && (t >= blockformats[blockindex + 1]->GetStartTime(object.object))) blockindex++;

      objparameters = blockformats[blockindex]->GetObjectParameters();
      objparametersvalid = true;
    }
    else objparametersvalid = false;

    if ((objectindex != oldobjectindex) || (blockindex != oldblockindex))
    {
      DEBUG4(("Cursor<%016lx:%u>: Moved to object %u/%u ('%s'), block %u/%u at %0.3lfs (parameters '%s')", (ulong_t)this, channel, objectindex, (uint_t)objectlist.size(), objectlist[objectindex].object->ToString().c_str(), blockindex, (uint_t)objectlist[objectindex].channelformat->GetBlockFormatRefs().size(), (double)t * 1.0e-9, objectlist[objectindex].channelformat->GetBlockFormatRefs()[blockindex]->ToString().c_str()));
    }
  }

  currenttime = t;

  return ((objectindex != oldobjectindex) || (blockindex != oldblockindex));
}

BBC_AUDIOTOOLBOX_END
