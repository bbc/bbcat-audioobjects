
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include <map>
#include <algorithm>

#define DEBUG_LEVEL 1
#include <aplibs-dsp/ByteSwap.h>

#include "ADMObjects.h"
#include "ADMData.h"

BBC_AUDIOTOOLBOX_START

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
                                                                                          name(_name)
{
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
void ADMObject::SetValue(ADMVALUE& obj, const std::string& name, uint_t value, bool attr)
{
  std::string valuestr;
  Printf(valuestr, "%u", value);
  SetValue(obj, name, valuestr, attr);
}
void ADMObject::SetValue(ADMVALUE& obj, const std::string& name, uint64_t value, bool attr)
{
  SetValue(obj, name, GenTime(value), attr);
}
void ADMObject::SetValue(ADMVALUE& obj, const std::string& name, double value, bool attr)
{
  std::string valuestr;
  Printf(valuestr, "%0.6lf", value);
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
void ADMObject::SetValueAttribute(ADMVALUE& obj, const std::string& name, uint_t value)
{
  std::string valuestr;
  Printf(valuestr, "%u", value);
  obj.attrs[name] = valuestr;
}
void ADMObject::SetValueAttribute(ADMVALUE& obj, const std::string& name, uint64_t value)
{
  obj.attrs[name] = GenTime(value);
}
void ADMObject::SetValueAttribute(ADMVALUE& obj, const std::string& name, double value)
{
  std::string valuestr;
  Printf(valuestr, "%0.6lf", value);
  obj.attrs[name] = valuestr;
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetValues()
{
  SetValue(typeLabel, "typeLabel");
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
  if (full || (typeLabel != ""))
  {
    SetAttribute(value, "typeLabel", typeLabel);
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
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::EraseValue(const ADMVALUE *value)
{
  ADMVALUES::iterator it;
    
  for (it = values.begin(); it != values.end(); ++it)
  {
    const ADMVALUE& value1 = *it;

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
 *
 * @return true if value found and variable set
 *
 * @note the value, if found, is REMOVED from the list
 */
/*--------------------------------------------------------------------------------*/
bool ADMObject::SetValue(uint_t& res, const std::string& name)
{
  const ADMVALUE *value;
  bool success = false;

  if ((value = GetValue(name)) != NULL)
  {
    success = (sscanf(value->value.c_str(), "%u", &res) > 0);
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
bool ADMObject::SetValue(ulong_t& res, const std::string& name)
{
  const ADMVALUE *value;
  bool success = false;

  if ((value = GetValue(name)) != NULL)
  {
    success = (sscanf(value->value.c_str(), "%lu", &res) > 0);
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
bool ADMObject::SetValue(sint_t& res, const std::string& name)
{
  const ADMVALUE *value;
  bool success = false;

  if ((value = GetValue(name)) != NULL)
  {
    success = (sscanf(value->value.c_str(), "%d", &res) > 0);
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
bool ADMObject::SetValue(slong_t& res, const std::string& name)
{
  const ADMVALUE *value;
  bool success = false;

  if ((value = GetValue(name)) != NULL)
  {
    success = (sscanf(value->value.c_str(), "%ld", &res) > 0);
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
  ADMTimeObject(),
  startTime(0),
  duration(0),
  childrenMinChannel(~0),
  childrenMaxChannel(0)
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
    childrenMinChannel = MIN(childrenMinChannel, obj->GetTrackNum() - 1);
    childrenMaxChannel = MAX(childrenMaxChannel, obj->GetTrackNum() - 1);
    return true;
  }

  // reference is already in the list
  return true;
}

void ADMAudioObject::UpdateLimits()
{
  uint_t i;

  for (i = 0; i < trackrefs.size(); i++)
  {
    uint_t t = trackrefs[i]->GetTrackNum() - 1;
    childrenMinChannel = MIN(childrenMinChannel, t);
    childrenMaxChannel = MAX(childrenMaxChannel, t);
  }

  for (i = 0; i < packformatrefs.size(); i++)
  {
    ADMAudioPackFormat *obj = packformatrefs[i];

    obj->UpdateLimits();
    Update(obj);
  }

  for (i = 0; i < trackrefs.size(); i++)
  {
    ADMAudioTrack *obj = trackrefs[i];

    obj->UpdateLimits();
    Update(obj);
  }

  DEBUG4(("%s start %s end %s",
          ToString().c_str(),
          GenTime(GetChildrenStartTime()).c_str(),
          GenTime(GetChildrenEndTime()).c_str()));
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

  if (full)
  {
    // full (non-XML) list
    SetAttribute(value, "blockChannelStart", GetChildrenStartChannel());
    objvalues.push_back(value);

    SetAttribute(value, "blockChannelCount", GetChildrenChannelCount());
    objvalues.push_back(value);

    SetAttribute(value, "blockStartTime",    GetChildrenStartTime());
    objvalues.push_back(value);

    SetAttribute(value, "blockEndTime",      GetChildrenEndTime());
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

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrack::SetValues()
{
  ADMObject::SetValues();

  SetValue(trackNum,   "trackNum");
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
    SetAttribute(value, "trackNum", trackNum); objvalues.push_back(value);
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
/** Update time limits
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrack::UpdateLimits()
{
  uint_t i;

  for (i = 0; i < trackformatrefs.size(); i++)
  {
    ADMAudioTrackFormat *obj = trackformatrefs[i];

    obj->UpdateLimits();
    Update(obj);
  }

  for (i = 0; i < packformatrefs.size(); i++)
  {
    ADMAudioPackFormat *obj = packformatrefs[i];

    obj->UpdateLimits();
    Update(obj);
  }
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

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioPackFormat::SetValues()
{
  ADMObject::SetValues();
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

  if (full)
  {
    SetAttribute(value, "startTime", GetChildrenStartTime());
    objvalues.push_back(value);

    SetAttribute(value, "endTime",   GetChildrenEndTime());
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
/** Update time limits
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioPackFormat::UpdateLimits()
{
  uint_t i;

  for (i = 0; i < channelformatrefs.size(); i++)
  {
    ADMAudioChannelFormat *obj = channelformatrefs[i];

    obj->UpdateLimits();
    Update(obj);
  }
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

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::SetValues()
{
  ADMObject::SetValues();

  SetValue(formatLabel,      "formatLabel");
  SetValue(formatDefinition, "formatDefinition");
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
/** Update time limits
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::UpdateLimits()
{
  uint_t i;

  for (i = 0; i < channelformatrefs.size(); i++)
  {
    ADMAudioChannelFormat *obj = channelformatrefs[i];

    obj->UpdateLimits();
    Update(obj);
  }

  for (i = 0; i < packformatrefs.size(); i++)
  {
    ADMAudioPackFormat *obj = packformatrefs[i];

    obj->UpdateLimits();
    Update(obj);
  }
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
  if (full || (formatLabel != ""))
  {
    SetAttribute(value, "formatLabel", formatLabel);
    objvalues.push_back(value);
  }
  if (full || (formatDefinition != ""))
  {
    SetAttribute(value, "formatDefinition", formatDefinition);
    objvalues.push_back(value);
  }

  if (full)
  {
    SetAttribute(value, "startTime", GetChildrenStartTime());
    objvalues.push_back(value);

    SetAttribute(value, "endTime",   GetChildrenEndTime());
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

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrackFormat::SetValues()
{
  ADMObject::SetValues();

  SetValue(formatLabel,      "formatLabel");
  SetValue(formatDefinition, "formatDefinition");
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
  if (full || (formatLabel != ""))
  {
    SetAttribute(value, "formatLabel", formatLabel);
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
/** Update time limits
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrackFormat::UpdateLimits()
{
  uint_t i;

  for (i = 0; i < streamformatrefs.size(); i++)
  {
    ADMAudioStreamFormat *obj = streamformatrefs[i];

    obj->UpdateLimits();
    Update(obj);
  }
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

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioChannelFormat::SetValues()
{
  ADMObject::SetValues();
}

/*--------------------------------------------------------------------------------*/
/** Add reference to an AudioBlockFormat object and ensures blocks are sorted by time
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioChannelFormat::Add(ADMAudioBlockFormat *obj)
{
  if (std::find(blockformatrefs.begin(), blockformatrefs.end(), obj) == blockformatrefs.end())
  {
    blockformatrefs.push_back(obj);
    sort(blockformatrefs.begin(), blockformatrefs.end(), ADMAudioBlockFormat::Compare);

    Update(obj->GetStartTime(), obj->GetEndTime());
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

  if (full)
  {
    SetAttribute(value, "startTime", GetChildrenStartTime());
    objvalues.push_back(value);

    SetAttribute(value, "endTime",   GetChildrenEndTime());
    objvalues.push_back(value);
  }

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
    else if (value.name == "diffuse")
    {
      supplement.Set(value.name, (value.value == "true"));

      it = values.erase(it);
    }
    else ++it;
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

  bool diffuse;
  if (supplement.Get("diffuse", diffuse))
  {
    SetValue(value, "diffuse", diffuse ? "true" : "false");
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

ADMTrackCursor::ADMTrackCursor(const ADMAudioTrack *_track) : PositionCursor(),
                                                              track(NULL),
                                                              channelformat(NULL),
                                                              blockformats(NULL),
                                                              currenttime(0),
                                                              blockindex(0)
{
  if (_track) Setup(_track);
}

ADMTrackCursor::ADMTrackCursor(const ADMTrackCursor& obj) : PositionCursor(),
                                                            track(obj.track),
                                                            channelformat(obj.channelformat),
                                                            blockformats(obj.blockformats),
                                                            currenttime(obj.currenttime),
                                                            blockindex(obj.blockindex)
{
}

ADMTrackCursor::~ADMTrackCursor()
{
}

void ADMTrackCursor::Setup(const ADMAudioTrack *_track)
{
  blockformats = NULL;
  blockindex   = 0;

  if ((track = _track) != NULL)
  {
    const ADMAudioTrackFormat *trackformat;

    if ((track->GetTrackFormatRefs().size() > 0) && ((trackformat = track->GetTrackFormatRefs().front()) != NULL))
    {
      const ADMAudioStreamFormat *streamformat;

      if ((trackformat->GetStreamFormatRefs().size() > 0) && ((streamformat = trackformat->GetStreamFormatRefs().front()) != NULL))
      {
        if ((streamformat->GetChannelFormatRefs().size() > 0) && ((channelformat = const_cast<ADMAudioChannelFormat *>(streamformat->GetChannelFormatRefs().front())) != NULL))
        {
          blockformats = &channelformat->GetBlockFormatRefs();
        }
        else ERROR("Failed to find channel format for '%s' (track '%s')", streamformat->ToString().c_str(), track->ToString().c_str());
      }
      else ERROR("Failed to find stream format for '%s' (track '%s')", trackformat->ToString().c_str(), track->ToString().c_str());
    }
    else ERROR("Failed to find track format for track '%s'", track->ToString().c_str());
  }
}

ADMTrackCursor& ADMTrackCursor::operator = (const ADMTrackCursor& obj)
{
  track         = obj.track;
  channelformat = obj.channelformat;
  blockformats  = obj.blockformats;
  currenttime   = obj.currenttime;
  blockindex    = obj.blockindex;
  return *this;
}

/*--------------------------------------------------------------------------------*/
/** Return position at current time
 */
/*--------------------------------------------------------------------------------*/
const Position *ADMTrackCursor::GetPosition() const
{
  const Position *res = NULL;

  if (blockformats && (blockindex < blockformats->size())) res = &(*blockformats)[blockindex]->GetPosition();

  return res;
}

/*--------------------------------------------------------------------------------*/
/** Return supplementary information at current time
 */
/*--------------------------------------------------------------------------------*/
const ParameterSet *ADMTrackCursor::GetPositionSupplement() const
{
  const ParameterSet *res = NULL;

  if (blockformats && (blockindex < blockformats->size())) res = &(*blockformats)[blockindex]->GetPositionSupplement();

  return res;
}

/*--------------------------------------------------------------------------------*/
/** Set position for current time
 */
/*--------------------------------------------------------------------------------*/
void ADMTrackCursor::SetPosition(const Position& pos, const ParameterSet *supplement)
{
  if (blockformats)
  {
    const Position     *current_pos        = GetPosition();
    const ParameterSet *current_supplement = GetPositionSupplement();

    // decide whether a new blockformat is required
    if (!current_pos                                                                || // if no existing entries; or
        (current_pos && (pos != *current_pos))                                      || // if position has changed; or
        (supplement  && !current_supplement)                                        || // a supplement is supplied when no current one exists; or
        (!supplement && current_supplement)                                         || // no supplement is suppled when current one does exist; or
        (supplement  && current_supplement && (*supplement != *current_supplement)))   // supplement has changed
    {
      ADMData&            adm = channelformat->GetOwner();
      ADMAudioBlockFormat *blockformat     = NULL;
      bool                newblockrequired = true;

      // close current one off by setting end time
      if (blockindex < blockformats->size())
      {
        blockformat = (*blockformats)[blockindex];

        if (blockformat->GetRTime() == currenttime)
        {
          // new position at same time as original -> just update this position
          blockformat->SetPosition(pos, supplement);
          // no need to create new blockformat
          newblockrequired = false;
        }
        else blockformat->SetDuration(currenttime - blockformat->GetRTime());
      }

      // create new blockformat if required
      if (newblockrequired && ((blockformat = new ADMAudioBlockFormat(adm, adm.CreateID(ADMAudioBlockFormat::Type, channelformat), "")) != NULL))
      {
        blockformat->SetRTime(currenttime);
        blockformat->SetPosition(pos, supplement);
        channelformat->Add(blockformat);
        Seek(currenttime);
      }

      if (blockformat) DEBUG2(("Set position to %s at %lu", blockformat->GetPosition().ToString().c_str(), (ulong_t)currenttime));
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** End position updates by marking the end of the last block
 */
/*--------------------------------------------------------------------------------*/
void ADMTrackCursor::EndPositionChanges()
{
  // close last block format off by setting end time
  if (blockformats)
  {
    if (blockformats->size() > 0)
    {
      ADMAudioBlockFormat *blockformat = blockformats->back();

      blockformat->SetDuration(currenttime - blockformat->GetRTime());

      DEBUG2(("Set duration of last block at %lu to %lu", (ulong_t)currenttime, (ulong_t)blockformat->GetDuration()));
    }
    else
    {
      ADMData&            adm = channelformat->GetOwner();
      ADMAudioBlockFormat *blockformat;

      if ((blockformat = adm.CreateBlockFormat("", channelformat)) != NULL)
      {
        blockformat->SetDuration(currenttime - blockformat->GetRTime());
        DEBUG2(("Created empty block with duration at %lu of %lu", (ulong_t)currenttime, (ulong_t)blockformat->GetDuration()));
      }
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Get position at specified time (ns)
 */
/*--------------------------------------------------------------------------------*/
bool ADMTrackCursor::Seek(uint64_t t)
{
  uint_t oldindex = blockindex;

  if (blockformats)
  {
    size_t n = blockformats->size();

    // move blockindex to point to the correct index
    while ((blockindex       > 0) && (t <  (*blockformats)[blockindex]->GetRTime()))     blockindex--;
    while (((blockindex + 1) < n) && (t >= (*blockformats)[blockindex + 1]->GetRTime())) blockindex++;
  }

  currenttime = t;

  return (blockindex != oldindex);
}

BBC_AUDIOTOOLBOX_END
