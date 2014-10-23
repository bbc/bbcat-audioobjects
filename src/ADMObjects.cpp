
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include <map>
#include <algorithm>

#define DEBUG_LEVEL 1
#include <bbcat-base/ByteSwap.h>

#include "ADMObjects.h"
#include "ADMData.h"

BBC_AUDIOTOOLBOX_START

std::map<uint_t,std::string> ADMObject::typeLabelMap;
std::map<uint_t,std::string> ADMObject::formatLabelMap;

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
                                                                                          typeLabel(0)
{
  if (typeLabelMap.size() == 0)
  {
    // populate typeLabel map
    SetTypeDefinition(TypeLabel_DirectSpeakers, "DirectSpeakers");
    SetTypeDefinition(TypeLabel_Matrix,         "Matrix");
    SetTypeDefinition(TypeLabel_Objects,        "Object");
    SetTypeDefinition(TypeLabel_HOA,            "HOA");
    SetTypeDefinition(TypeLabel_Binaural,       "Binaural");
  }

  SetTypeLabel(TypeLabel_DirectSpeakers);
}

/*--------------------------------------------------------------------------------*/
/** Set and Get object ID
 *
 * @param id new ID
 *
 * @note setting the ID updates the map held within the ADMData object
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetID(const std::string& _id)
{
  // get owner to change it and update its map of objects in the process
  owner.ChangeID(this, _id);
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
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetTypeLabel(uint_t type)
{
  typeLabel = type;

  // update typeDefinition if possible
  if (typeLabelMap.find(typeLabel) != typeLabelMap.end()) SetTypeDefinition(typeLabelMap[typeLabel]);
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
/** Set ADMVALUE object as a XML value (name/value pair) with optional attributes
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetValue(ADMVALUE& obj, const std::string& name, const std::string& value, bool attr)
{
  obj.attr  = attr;
  obj.name  = name;
  obj.value = value;
  obj.attrs.clear();
}
void ADMObject::SetValue(ADMVALUE& obj, const std::string& name, uint_t value, bool attr, const char *format)
{
  std::string valuestr;
  Printf(valuestr, format, value);
  SetValue(obj, name, valuestr, attr);
}
void ADMObject::SetValue(ADMVALUE& obj, const std::string& name, uint64_t value, bool attr)
{
  SetValue(obj, name, GenTime(value), attr);
}
void ADMObject::SetValue(ADMVALUE& obj, const std::string& name, double value, bool attr, const char *format)
{
  std::string valuestr;
  Printf(valuestr, format, value);
  SetValue(obj, name, valuestr, attr);
}

/*--------------------------------------------------------------------------------*/
/** Set attribute of ADMVALUE value object (initialised above)
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetValueAttribute(ADMVALUE& obj, const std::string& name, const std::string& value)
{
  obj.attrs[name] = value;
}
void ADMObject::SetValueAttribute(ADMVALUE& obj, const std::string& name, uint_t value, const char *format)
{
  std::string valuestr;
  Printf(valuestr, format, value);
  obj.attrs[name] = valuestr;
}
void ADMObject::SetValueAttribute(ADMVALUE& obj, const std::string& name, uint64_t value)
{
  obj.attrs[name] = GenTime(value);
}
void ADMObject::SetValueAttribute(ADMVALUE& obj, const std::string& name, double value, const char *format)
{
  std::string valuestr;
  Printf(valuestr, format, value);
  obj.attrs[name] = valuestr;
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetValues()
{
  SetValue(typeLabel,      "typeLabel", true);
  SetValue(typeDefinition, "typeDefinition");
}

/*--------------------------------------------------------------------------------*/
/** Try to connect references after all objects have been set up
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetReferences()
{
  ADMVALUES::iterator it;

  // cycle through values looking for references to the specified object type
  for (it = values.begin(); (it != values.end());)
  {
    const ADMObject *obj  = NULL; // for neater response handling
    const ADMVALUE& value = *it;
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
    else if (value.name == ADMAudioBlockFormat::Reference)
    {
      ADMAudioBlockFormat *ref;

      if ((ref = dynamic_cast<ADMAudioBlockFormat *>(owner.GetReference(value))) != NULL)
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
void ADMObject::GetValuesAndReferences(ADMVALUES& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  ADMVALUES::const_iterator it;
  ADMVALUE value;

  UNUSED_PARAMETER(objects);
  UNUSED_PARAMETER(full);

  if (GetID() != "")
  {
    if (GetType() == ADMAudioTrack::Type) SetAttribute(value, "UID",            GetID());
    else                                  SetAttribute(value, GetType() + "ID", GetID());
    objvalues.push_back(value);
  }

  if (GetName() != "")
  {
    SetAttribute(value, GetType() + "Name", GetName());
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
    SetAttribute(value, "typeLabel", typeLabel, "%04x");
    objvalues.push_back(value);
  }
  if (full || (typeDefinition != ""))
  {
    SetAttribute(value, "typeDefinition", typeDefinition);
    objvalues.push_back(value);
  }
}

/*--------------------------------------------------------------------------------*/
/** Add a value to the internal list
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::AddValue(const ADMVALUE& value)
{
  values.push_back(value);
}

/*--------------------------------------------------------------------------------*/
/** Return ptr to value with specified name or NULL
 */
/*--------------------------------------------------------------------------------*/
const ADMObject::ADMVALUE* ADMObject::GetValue(const std::string& name) const
{
  const ADMVALUE *value = NULL;
  uint_t i;

  // simple search and compare
  // (MUST use a list because there can be MULTIPLE values of the same name)
  for (i = 0; i < values.size(); i++)
  {
    if (values[i].name == name)
    {
      value = &values[i];
      break;
    }
  }

  if (!value) DEBUG4(("No value named '%s'!", name.c_str()));

  return value;
}

/*--------------------------------------------------------------------------------*/
/** Return ptr to attributes within specified value with specified name or NULL
 */
/*--------------------------------------------------------------------------------*/
const std::string *ADMObject::GetAttribute(const ADMVALUE& value, const std::string& name) const
{
  ADMATTRS::const_iterator it;
  const std::string *attr = NULL;

  if ((it = value.attrs.find(name)) != value.attrs.end())
  {
    attr = &it->second;
  }

  return attr;
}

/*--------------------------------------------------------------------------------*/
/** Remove and delete value from internal list of values
 *
 * @param value address of value to be erased
 *
 * @note value passed MUST be the address of the desired item
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::EraseValue(const ADMVALUE *value)
{
  ADMVALUES::iterator it;
    
  for (it = values.begin(); it != values.end(); ++it)
  {
    const ADMVALUE& value1 = *it;

    // note address comparison not value comparison!
    if (value == &value1)
    {
      values.erase(it);
      break;
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from value with specified name
 *
 * @param res internal variable to be modified
 * @param name value name
 *
 * @return true if value found and variable set
 *
 * @note the value, if found, is REMOVED from the list
 */
/*--------------------------------------------------------------------------------*/
bool ADMObject::SetValue(std::string& res, const std::string& name)
{
  const ADMVALUE *value;
  bool success = false;

  if ((value = GetValue(name)) != NULL)
  {
    res = value->value;
    EraseValue(value);
    success = true;
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from value with specified name
 *
 * @param res internal variable to be modified
 * @param name value name
 *
 * @return true if value found and variable set
 *
 * @note the value, if found, is REMOVED from the list
 */
/*--------------------------------------------------------------------------------*/
bool ADMObject::SetValue(double& res, const std::string& name)
{
  const ADMVALUE *value;
  bool success = false;

  if ((value = GetValue(name)) != NULL)
  {
    success = (sscanf(value->value.c_str(), "%lf", &res) > 0);
    EraseValue(value);
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from value with specified name
 *
 * @param res internal variable to be modified
 * @param name value name
 * @param hex true if value is expected to be hexidecimal
 *
 * @return true if value found and variable set
 *
 * @note the value, if found, is REMOVED from the list
 */
/*--------------------------------------------------------------------------------*/
bool ADMObject::SetValue(uint_t& res, const std::string& name, bool hex)
{
  const ADMVALUE *value;
  bool success = false;

  if ((value = GetValue(name)) != NULL)
  {
    if (hex) success = (sscanf(value->value.c_str(), "%x", &res) > 0);
    else     success = (sscanf(value->value.c_str(), "%u", &res) > 0);

    EraseValue(value);
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from value with specified name
 *
 * @param res internal variable to be modified
 * @param name value name
 *
 * @return true if value found and variable set
 *
 * @note the value, if found, is REMOVED from the list
 */
/*--------------------------------------------------------------------------------*/
bool ADMObject::SetValue(ulong_t& res, const std::string& name, bool hex)
{
  const ADMVALUE *value;
  bool success = false;

  if ((value = GetValue(name)) != NULL)
  {
    if (hex) success = (sscanf(value->value.c_str(), "%lx", &res) > 0);
    else     success = (sscanf(value->value.c_str(), "%lu", &res) > 0);
    EraseValue(value);
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from value with specified name
 *
 * @param res internal variable to be modified
 * @param name value name
 * @param hex true if value is expected to be hexidecimal
 *
 * @return true if value found and variable set
 *
 * @note the value, if found, is REMOVED from the list
 */
/*--------------------------------------------------------------------------------*/
bool ADMObject::SetValue(sint_t& res, const std::string& name, bool hex)
{
  const ADMVALUE *value;
  bool success = false;

  if ((value = GetValue(name)) != NULL)
  {
    if (hex) success = (sscanf(value->value.c_str(), "%x", (uint_t *)&res) > 0);
    else     success = (sscanf(value->value.c_str(), "%d", &res) > 0);
    EraseValue(value);
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from value with specified name
 *
 * @param res internal variable to be modified
 * @param name value name
 * @param hex true if value is expected to be hexidecimal
 *
 * @return true if value found and variable set
 *
 * @note the value, if found, is REMOVED from the list
 */
/*--------------------------------------------------------------------------------*/
bool ADMObject::SetValue(slong_t& res, const std::string& name, bool hex)
{
  const ADMVALUE *value;
  bool success = false;

  if ((value = GetValue(name)) != NULL)
  {
    if (hex) success = (sscanf(value->value.c_str(), "%lx", (ulong_t *)&res) > 0);
    else     success = (sscanf(value->value.c_str(), "%ld", &res) > 0);
    EraseValue(value);
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from value with specified name
 *
 * @param res internal variable to be modified
 * @param name value name
 *
 * @return true if value found and variable set
 *
 * @note the value, if found, is REMOVED from the list
 */
/*--------------------------------------------------------------------------------*/
bool ADMObject::SetValue(bool& res, const std::string& name)
{
  const ADMVALUE *value;
  bool success = false;

  if ((value = GetValue(name)) != NULL)
  {
    res = (value->value == "true");
    EraseValue(value);
    success = true;
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from value with specified name
 *
 * @param res internal variable to be modified
 * @param name value name
 *
 * @return true if value found and variable set
 *
 * @note the value, if found, is REMOVED from the list
 */
/*--------------------------------------------------------------------------------*/
bool ADMObject::SetValueTime(uint64_t& res, const std::string& name)
{
  const ADMVALUE *value;
  bool success = false;

  if ((value = GetValue(name)) != NULL)
  {
    success = CalcTime(res, value->value);
    EraseValue(value);
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Return ns-resolution time from textual hh:mm:ss.SSSSS
 *
 * @param str time in ASCII format
 *
 * @return ns time
 */
/*--------------------------------------------------------------------------------*/
bool ADMObject::CalcTime(uint64_t& t, const std::string& str)
{
  uint_t hr, mn, s, ss;
  bool   success = false;

  if (sscanf(str.c_str(), "%u:%u:%u.%u", &hr, &mn, &s, &ss) == 4)
  {
    t = hr;                 // hours
    t = (t * 60) + mn;      // minutes
    t = (t * 60) + s;       // seconds
    t = (t * 100000) + ss;  // 100000ths of second
    t *= 10000;             // nanoseconds
    success = true;
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Convert ns time to text time
 *
 * @param t ns time
 *
 * @return str time in text format 'hh:mm:ss.SSSSS'
 */
/*--------------------------------------------------------------------------------*/
std::string ADMObject::GenTime(uint64_t t)
{
  std::string str;
  uint_t hr, mn, s, ss;

  t /= 10000;
  ss = (uint_t)(t % 100000);
  t /= 100000;
  s  = (uint_t)(t % 60);
  t /= 60;
  mn = (uint_t)(t % 60);
  t /= 60;
  hr = (uint_t)t;

  Printf(str, "%02u:%02u:%02u.%05u", hr, mn, s, ss);

  return str;
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

  SetValue(language, "language");
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
void ADMAudioProgramme::GetValuesAndReferences(ADMVALUES& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  ADMVALUE value;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // add values/attributes not held in 'values' to list
  if (full || (language != ""))
  {
    SetAttribute(value, "language", language);
    objvalues.push_back(value);
  }

  // references only
  object.genref  = true;
  object.gendata = false;

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

  SetValue(language, "language");
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
void ADMAudioContent::GetValuesAndReferences(ADMVALUES& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  ADMVALUE value;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // add values/attributes not held in 'values' to list
  if (full || (language != ""))
  {
    SetAttribute(value, "language", language);
    objvalues.push_back(value);
  }

  // references only
  object.genref  = true;
  object.gendata = false;

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
  ADMLevelObject(),
  startTime(0),
  duration(0)
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

  SetValueTime(startTime, "startTime");
  SetStartTime(startTime);
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
    return true;
  }

  // reference is already in the list
  return true;
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioObject::GetValuesAndReferences(ADMVALUES& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  ADMVALUE value;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // add values/attributes not held in 'values' to list
  if (startTime)
  {
    SetAttribute(value, "startTime", startTime);
    objvalues.push_back(value);
  }
  if (duration)
  {
    SetAttribute(value, "duration",  duration);
    objvalues.push_back(value);
  }

  // references only
  object.genref  = true;
  object.gendata = false;

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
  if (SetValue(trackNum, "trackNum")) trackNum--;
  SetValue(sampleRate, "sampleRate");
  SetValue(bitDepth,   "bitDepth");
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrack::GetValuesAndReferences(ADMVALUES& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  ADMVALUE value;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // add values/attributes not held in 'values' to list
  if (full)
  {
    SetAttribute(value, "trackNum", trackNum + 1); objvalues.push_back(value);
  }
  SetAttribute(value, "sampleRate", sampleRate); objvalues.push_back(value);
  SetAttribute(value, "bitDepth",   bitDepth);   objvalues.push_back(value);

  // only record references for later output
  object.genref  = false;
  object.gendata = false;

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

  SetID(GetIDPrefix() + _id);
}

/*--------------------------------------------------------------------------------*/
/** Add reference to an AudioChannelFormat object
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioPackFormat::Add(ADMAudioChannelFormat *obj)
{
  if (std::find(channelformatrefs.begin(), channelformatrefs.end(), obj) == channelformatrefs.end())
  {
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
void ADMAudioPackFormat::GetValuesAndReferences(ADMVALUES& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  ADMVALUE         value;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // generate references only
  object.genref  = true;
  object.gendata = false;

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
    packformatrefs.push_back(obj);
    obj->Add(this);
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

  SetValue(formatLabel,      "formatLabel", true);
  SetValue(formatDefinition, "formatDefinition");
}

/*--------------------------------------------------------------------------------*/
/** Update object's ID
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::UpdateID()
{
  // call SetID() with new ID
  std::string _id;

  Printf(_id, "%04x%%04x", formatLabel);

  SetID(GetIDPrefix() + _id);
}

/*--------------------------------------------------------------------------------*/
/** Add reference to an AudioChannelFormat object
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioStreamFormat::Add(ADMAudioChannelFormat *obj)
{
  if (channelformatrefs.size() == 0)
  {
    channelformatrefs.push_back(obj);
    return true;
  }

  // only a single reference allowed
  return (std::find(channelformatrefs.begin(), channelformatrefs.end(), obj) != channelformatrefs.end());
}

/*--------------------------------------------------------------------------------*/
/** Add reference to an AudioTrackFormat object
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioStreamFormat::Add(ADMAudioTrackFormat *obj)
{
  if (std::find(trackformatrefs.begin(), trackformatrefs.end(), obj) == trackformatrefs.end())
  {
    trackformatrefs.push_back(obj);
    obj->Add(this);
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
    packformatrefs.push_back(obj);
    return true;
  }

  // only a single reference allowed
  return (std::find(packformatrefs.begin(), packformatrefs.end(), obj) != packformatrefs.end());
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::GetValuesAndReferences(ADMVALUES& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  ADMVALUE value;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // add values/attributes not held in 'values' to list
  if (full || (formatLabel != 0))
  {
    SetAttribute(value, "formatLabel", formatLabel, "%04x");
    objvalues.push_back(value);
  }
  if (full || (formatDefinition != ""))
  {
    SetAttribute(value, "formatDefinition", formatDefinition);
    objvalues.push_back(value);
  }

  // generate references only
  object.genref  = true;
  object.gendata = false;

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

  SetValue(formatLabel,      "formatLabel", true);
  SetValue(formatDefinition, "formatDefinition");
}

/*--------------------------------------------------------------------------------*/
/** Update object's ID
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrackFormat::UpdateID()
{
  // call SetID() with new ID
  std::string _id;

  if (formatLabel) Printf(_id, "%04x%04x_%%02x", typeLabel, formatLabel);
  else             Printf(_id, "%04x0001_%%02x", typeLabel);

  SetID(GetIDPrefix() + _id);
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrackFormat::GetValuesAndReferences(ADMVALUES& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  ADMVALUE value;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // add values/attributes not held in 'values' to list
  if (full || (formatLabel != 0))
  {
    SetAttribute(value, "formatLabel", formatLabel, "%04x");
    objvalues.push_back(value);
  }
  if (full || (formatDefinition != ""))
  {
    SetAttribute(value, "formatDefinition", formatDefinition);
    objvalues.push_back(value);
  }

  // generate references only
  object.genref  = true;
  object.gendata = false;

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
    streamformatrefs.push_back(obj);
    obj->Add(this);
    return true;
  }

  // only a single reference allowed
  return (std::find(streamformatrefs.begin(), streamformatrefs.end(), obj) != streamformatrefs.end());
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

  SetID(GetIDPrefix() + _id);
}

/*--------------------------------------------------------------------------------*/
/** Add reference to an AudioBlockFormat object and ensures blocks are sorted by time
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioChannelFormat::Add(ADMAudioBlockFormat *obj)
{
  if (std::find(blockformatrefs.begin(), blockformatrefs.end(), obj) == blockformatrefs.end())
  {
    obj->SetChannelFormat(this);
    blockformatrefs.push_back(obj);
    sort(blockformatrefs.begin(), blockformatrefs.end(), ADMAudioBlockFormat::Compare);

    return true;
  }

  // reference is already in the list
  return true;
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioChannelFormat::GetValuesAndReferences(ADMVALUES& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  REFERENCEDOBJECT object;
  ADMVALUE         value;
  uint_t i;

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // generate contained data entries
  object.genref  = false;
  object.gendata = true;

  for (i = 0; i < blockformatrefs.size(); i++)
  {
    object.obj = blockformatrefs[i];
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
void ADMAudioChannelFormat::GenerateReferenceList(std::string& str) const
{
  UNUSED_PARAMETER(str);
}

/*----------------------------------------------------------------------------------------------------*/

const std::string ADMAudioBlockFormat::Type      = "audioBlockFormat";
const std::string ADMAudioBlockFormat::Reference = Type + "IDRef";
const std::string ADMAudioBlockFormat::IDPrefix  = "AB_";


/*--------------------------------------------------------------------------------*/
/** Update object's ID
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioBlockFormat::UpdateID()
{
  if (channelformat)
  {
    // change ID to AB_yyyyxxxx_zzzzzzzz where yyyy and xxxx are taken from the owning channelcformat  object's ID and zzzzzzzz is auto-generated
    owner.ChangeID(this, GetIDPrefix() + channelformat->GetID().substr(channelformat->GetIDPrefix().length()) + "_%08x");
  }
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioBlockFormat::SetValues()
{
  ADMVALUES::iterator it;

  ADMObject::SetValues();

  SetValueTime(rtime, "rtime");
  SetValueTime(duration, "duration");

  for (it = values.begin(); it != values.end();)
  {
    const ADMVALUE& value = *it;

    if (value.name == "position")
    {
      double val;

      if (sscanf(value.value.c_str(), "%lf", &val) > 0)
      {
        const std::string *attr;

        if ((attr = GetAttribute(value, "coordinate")) != NULL)
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
    else // any other parameters -> assume they are part of the supplement information
    {
      supplement.Set(value.name, value.value);

      it = values.erase(it);
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Set position for this blockformat
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioBlockFormat::SetPosition(const Position& pos, const ParameterSet *supplement)
{
  position = pos;
  if (supplement) this->supplement = *supplement;
}

/*--------------------------------------------------------------------------------*/
/** Return list of values/attributes from internal variables and list of referenced objects
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioBlockFormat::GetValuesAndReferences(ADMVALUES& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full) const
{
  ADMVALUE value;

  UNUSED_PARAMETER(objects);

  // populate list from parent object
  ADMObject::GetValuesAndReferences(objvalues, objects, full);

  // add values/attributes not held in 'values' to list
  if (full || rtime)
  {
    SetAttribute(value, "rtime", rtime);
    objvalues.push_back(value);
  }
  if (full || duration)
  {
    SetAttribute(value, "duration", duration);
    objvalues.push_back(value);
  }

  if (position.polar)
  {
    SetValue(value, "position", position.pos.az);
    SetValueAttribute(value, "coordinate", "azimuth");
    objvalues.push_back(value);

    SetValue(value, "position", position.pos.el);
    SetValueAttribute(value, "coordinate", "elevation");
    objvalues.push_back(value);

    SetValue(value, "position", position.pos.d);
    SetValueAttribute(value, "coordinate", "distance");
    objvalues.push_back(value);
  }
  else
  {
    SetValue(value, "position", position.pos.x);
    SetValueAttribute(value, "coordinate", "x");
    objvalues.push_back(value);

    SetValue(value, "position", position.pos.y);
    SetValueAttribute(value, "coordinate", "y");
    objvalues.push_back(value);

    SetValue(value, "position", position.pos.z);
    SetValueAttribute(value, "coordinate", "z");
    objvalues.push_back(value);
  }

  // add all parameters from the supplement information
  ParameterSet::Iterator it;
  for (it = supplement.GetBegin(); it != supplement.GetEnd(); ++it)
  {
    SetValue(value, it->first, it->second);
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

/*----------------------------------------------------------------------------------------------------*/

ADMTrackCursor::ADMTrackCursor(uint_t _channel) : PositionCursor(),
                                                  channel(_channel),
                                                  objectindex(0),
                                                  blockindex(0),
                                                  currenttime(0),
                                                  blockformatstarted(false)
{
}

ADMTrackCursor::ADMTrackCursor(const ADMTrackCursor& obj) : PositionCursor(),
                                                            channel(obj.channel),
                                                            objectindex(0),
                                                            blockindex(0),
                                                            currenttime(0),
                                                            blockformatstarted(false)
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
  uint_t i;
  bool   added = false;

  // look for this object in the list
  for (i = 0; i < objectlist.size(); i++)
  {
    // if object is already in list, exit now
    if (objectlist[i].object == object) return false;
  }

  const std::vector<ADMAudioTrack *>& trackrefs = object->GetTrackRefs();
  for (i = 0; (i < trackrefs.size()) && !added; i++)
  {
    if (trackrefs[i]->GetTrackNum() == channel)
    {
      const std::vector<ADMAudioTrackFormat *>& trackformatrefs = trackrefs[i]->GetTrackFormatRefs();

      DEBUG5(("Cursor<%016lx:%u>: Object '%s' uses this track", (ulong_t)this, channel, object->ToString().c_str()));

      if (trackformatrefs.size() == 1)
      {
        const std::vector<ADMAudioStreamFormat *>& streamformatrefs = trackformatrefs[0]->GetStreamFormatRefs();

        if (streamformatrefs.size() == 1)
        {
          const std::vector<ADMAudioChannelFormat *>& channelformatrefs = streamformatrefs[0]->GetChannelFormatRefs();

          if (channelformatrefs.size() == 1)
          {
            AUDIOOBJECT obj =
            {
              object,
              channelformatrefs[0],
            };

            objectlist.push_back(obj);

            DEBUG3(("Cursor<%016lx:%u>: Added object '%s', %u blocks", (ulong_t)this, channel, object->ToString().c_str(), (uint_t)obj.channelformat->GetBlockFormatRefs().size()));

            added = true;
          }
          else ERROR("Incorrect channelformatrefs in '%s' (%u)!", streamformatrefs[0]->ToString().c_str(), (uint_t)channelformatrefs.size());
        }
        else ERROR("Incorrect streamformatrefs in '%s' (%u)!", trackformatrefs[0]->ToString().c_str(), (uint_t)streamformatrefs.size());
      }
      else ERROR("Incorrect trackformatrefs in '%s' (%u)!", trackrefs[0]->ToString().c_str(), (uint_t)trackformatrefs.size());
    }
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
  bool   added = false;
  uint_t i;

  for (i = 0; i < objects.size(); i++) added |= Add(objects[i], false);

  if (added) Sort();

  return added;
}


/*--------------------------------------------------------------------------------*/
/** Sort list of objects into time order
 */
/*--------------------------------------------------------------------------------*/
void ADMTrackCursor::Sort()
{
  std::sort(objectlist.begin(), objectlist.end(), &Compare);

  Seek(currenttime);
}

/*--------------------------------------------------------------------------------*/
/** Return cursor start time in ns
 */
/*--------------------------------------------------------------------------------*/
uint64_t ADMTrackCursor::GetStartTime() const
{
  uint64_t t = 0;

  if (objectlist.size() > 0)
  {
    const AUDIOOBJECT&                        object       = objectlist[0];
    const std::vector<ADMAudioBlockFormat *>& blockformats = objectlist[objectindex].channelformat->GetBlockFormatRefs();

    if (blockformats.size() > 0) t = object.object->GetStartTime() + blockformats[0]->GetStartTime();
  }

  return t;
}

/*--------------------------------------------------------------------------------*/
/** Return cursor end time in ns
 */
/*--------------------------------------------------------------------------------*/
uint64_t ADMTrackCursor::GetEndTime() const
{
  uint64_t t = 0;

  if (objectlist.size() > 0)
  {
    const AUDIOOBJECT&                        object       = objectlist[0];
    const std::vector<ADMAudioBlockFormat *>& blockformats = objectlist[objectindex].channelformat->GetBlockFormatRefs();

    if (blockformats.size() > 0) t = object.object->GetStartTime() + blockformats[blockformats.size() - 1]->GetStartTime() + blockformats[blockformats.size() - 1]->GetDuration();
  }

  return t;
}

/*--------------------------------------------------------------------------------*/
/** Return position at current time
 */
/*--------------------------------------------------------------------------------*/
const Position *ADMTrackCursor::GetPosition() const
{
  const Position *pos = NULL;

  if (objectindex < objectlist.size())
  {
    const std::vector<ADMAudioBlockFormat *>& blockformats = objectlist[objectindex].channelformat->GetBlockFormatRefs();

    if (blockindex < blockformats.size()) pos = &blockformats[blockindex]->GetPosition();
  }

  return pos;
}

/*--------------------------------------------------------------------------------*/
/** Return supplementary information at current time
 */
/*--------------------------------------------------------------------------------*/
const ParameterSet *ADMTrackCursor::GetPositionSupplement() const
{
  const ParameterSet *supplement = NULL;

  if (objectindex < objectlist.size())
  {
    const std::vector<ADMAudioBlockFormat *>& blockformats = objectlist[objectindex].channelformat->GetBlockFormatRefs();

    if (blockindex < blockformats.size()) supplement = &blockformats[blockindex]->GetPositionSupplement();
  }

  return supplement;
}

/*--------------------------------------------------------------------------------*/
/** Start a blockformat at t
 */
/*--------------------------------------------------------------------------------*/
ADMAudioBlockFormat *ADMTrackCursor::StartBlockFormat(uint64_t t)
{
  AUDIOOBJECT&        object       = objectlist[objectindex];
  ADMData&            adm          = object.channelformat->GetOwner();
  ADMAudioBlockFormat *blockformat;
  uint64_t            relativetime = t - object.object->GetStartTime();

  if ((blockformat = new ADMAudioBlockFormat(adm, adm.CreateID(ADMAudioBlockFormat::Type), "")) != NULL)
  {
    blockformat->SetRTime(relativetime);
    object.channelformat->Add(blockformat);
    
    blockindex         = object.channelformat->GetBlockFormatRefs().size() - 1;
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
  if (blockformatstarted)
  {
    AUDIOOBJECT&        object       = objectlist[objectindex];
    ADMAudioBlockFormat *blockformat = object.channelformat->GetBlockFormatRefs()[blockindex];
    uint64_t            relativetime = t - object.object->GetStartTime();
  
    blockformat->SetDuration(relativetime - blockformat->GetStartTime());

    DEBUG3(("Cursor<%016lx:%u>: Completed blockformat %u at %0.3lfs (duration %0.3lfs) for object '%s', channelformat '%s'", (ulong_t)this, channel, blockindex, (double)t * 1.0e-9, (double)blockformat->GetDuration() * 1.0e-9, object.object->ToString().c_str(), object.channelformat->ToString().c_str()));

    blockformatstarted = false;
  }
}

/*--------------------------------------------------------------------------------*/
/** Set position for current time
 */
/*--------------------------------------------------------------------------------*/
void ADMTrackCursor::SetPosition(const Position& pos, const ParameterSet *supplement)
{
  if ((objectindex < objectlist.size()) && (currenttime >= objectlist[objectindex].object->GetStartTime()))
  {
    AUDIOOBJECT&                        object              = objectlist[objectindex];
    std::vector<ADMAudioBlockFormat *>& blockformats        = object.channelformat->GetBlockFormatRefs();
    const Position                      *current_pos        = GetPosition();
    const ParameterSet                  *current_supplement = GetPositionSupplement();

    // decide whether a new blockformat is required
    if (!current_pos                                                                || // if no existing entries; or
        (current_pos && (pos != *current_pos))                                      || // if position has changed; or
        (supplement  && !current_supplement)                                        || // a supplement is supplied when no current one exists; or
        (!supplement && current_supplement)                                         || // no supplement is suppled when current one does exist; or
        (supplement  && current_supplement && (*supplement != *current_supplement)))   // supplement has changed
    {
      ADMAudioBlockFormat *blockformat;
      uint64_t            relativetime = currenttime - object.object->GetStartTime();

      if ((blockindex < blockformats.size()) && (blockformats[blockindex]->GetStartTime() == relativetime))
      {
        // new position at same time as original -> just update this position
        blockformats[blockindex]->SetPosition(pos, supplement);
      }
      else
      {
        // new position requires new block format
        EndBlockFormat(currenttime);

        if ((blockformat = StartBlockFormat(currenttime)) != NULL)
        {
          blockformat->SetPosition(pos, supplement);
        }
      }
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** End position updates by marking the end of the last block
 */
/*--------------------------------------------------------------------------------*/
void ADMTrackCursor::EndPositionChanges()
{
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
      if (blockformats.size() > 0) blockindex = blockformats.size() - 1;
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
    const std::vector<ADMAudioBlockFormat *>& blockformats = objectlist[objectindex].channelformat->GetBlockFormatRefs();
    if ((t >= objectlist[objectindex].object->GetStartTime()) && (blockindex < blockformats.size()))
    {
      uint64_t rtime = t - objectlist[objectindex].object->GetStartTime();

      // find right blockformat within object
      while ((blockindex       > 0)                   && (rtime <  blockformats[blockindex]->GetStartTime()))     blockindex--;
      while (((blockindex + 1) < blockformats.size()) && (rtime >= blockformats[blockindex + 1]->GetStartTime())) blockindex++;
    }

    if ((objectindex != oldobjectindex) || (blockindex != oldblockindex))
    {
      DEBUG4(("Cursor<%016lx:%u>: Moved to object %u/%u ('%s'), block %u/%u at %0.3lfs", (ulong_t)this, channel, objectindex, (uint_t)objectlist.size(), objectlist[objectindex].object->ToString().c_str(), blockindex, (uint_t)objectlist[objectindex].channelformat->GetBlockFormatRefs().size(), (double)t * 1.0e-9));
    }
  }

  currenttime = t;

  return ((objectindex != oldobjectindex) || (blockindex != oldblockindex));
}

BBC_AUDIOTOOLBOX_END
