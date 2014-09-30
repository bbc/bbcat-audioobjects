
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
/** Output additional XML attributes for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::XMLAttributes(std::string& str) const
{
  ADMVALUES::const_iterator it;

  // typeLabel can appear in most objects, handle it here
  XMLAttribute(str, "typeLabel", typeLabel);
  XMLAttribute(str, "typeDefinition", typeDefinition);

  // output XML attribute values from list - this allows arbitary attributes to be stored
  for (it = values.begin(); it != values.end(); ++it)
  {
    const ADMVALUE& value = *it;

    if (value.attr) XMLAttribute(str, value.name, value.value);
  }
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML data for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::XMLData(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, std::vector<const ADMObject *>& reflist) const
{
  ADMVALUES::const_iterator it;

  UNUSED_PARAMETER(reflist);

  // this base object version will be called by every derived version

  // cycle through all values looking for non-attribute values and output them
  for (it = values.begin(); it != values.end(); ++it)
  {
    const ADMVALUE& value = *it;

    if (!value.attr)
    {
      ADMATTRS::const_iterator it2;

      // create new XML section for this value
      Printf(str, "%s<%s", CreateIndent(indent, ind_level + 1).c_str(), value.name.c_str());

      // add sub-attributes as XML attributes of this section
      for (it2 = value.attrs.begin(); it2 != value.attrs.end(); ++it2)
      {
        XMLAttribute(str, it2->first.c_str(), it2->second.c_str());
      }

      // if value is valid, output it and close the XML section
      if (value.value != "")
      {
        Printf(str, ">%s</%s>%s", value.value.c_str(), value.name.c_str(), eol.c_str());
      }
      // otherwise close the XML section directly
      else Printf(str, " />%s", eol.c_str());
    }
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

/*--------------------------------------------------------------------------------*/
/** Individual variable type dumping helper function
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::Dump(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, const std::string& name, const std::string& value)
{
  if (value != "")
  {
    Printf(str, "%s%s '%s'%s",
           CreateIndent(indent, ind_level + 1).c_str(),
           name.c_str(),
           value.c_str(),
           eol.c_str());
  }
}

/*--------------------------------------------------------------------------------*/
/** Individual variable type dumping helper function
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::Dump(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, const std::string& name, uint_t value)
{
  Printf(str, "%s%s %u%s",
         CreateIndent(indent, ind_level + 1).c_str(),
         name.c_str(),
         value,
         eol.c_str());
}

/*--------------------------------------------------------------------------------*/
/** Individual variable type dumping helper function
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::Dump(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, const std::string& name, bool value)
{
  Printf(str, "%s%s %s%s",
         CreateIndent(indent, ind_level + 1).c_str(),
         name.c_str(),
         value ? "true" : "false",
         eol.c_str());
}

/*--------------------------------------------------------------------------------*/
/** Individual variable type dumping helper function
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::Dump(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, const std::string& name, double value)
{
  Printf(str, "%s%s %0.6f%s",
         CreateIndent(indent, ind_level + 1).c_str(),
         name.c_str(),
         value,
         eol.c_str());
}

/*--------------------------------------------------------------------------------*/
/** Individual variable type dumping helper function
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::DumpTime(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, const std::string& name, uint64_t value)
{
  Printf(str, "%s%s '%s' (%luns)%s",
         CreateIndent(indent, ind_level + 1).c_str(),
         name.c_str(),
         GenTime(value).c_str(),
         (ulong_t)value,
         eol.c_str());
}

/*--------------------------------------------------------------------------------*/
/** Dump (in text form) information about this and reference objects
 *
 * @param str string to be modified
 * @param indent a string representing one level of indentation (e.g. a tab or spaces)
 * @param eol a string representing the end of line string
 * @param ind_level the initial indentation level
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::Dump(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const
{
  std::map<const ADMObject *,bool> handledmap;     // ensures each object is output ONLY once since references can be circular

  Dump(handledmap, str, indent, eol, ind_level);
}

/*--------------------------------------------------------------------------------*/
/** A handler function for the above - DO NOT CALL THIS DIRECTLY, call the above
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::Dump(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const
{
  // if this object has not been outputted previously
  if (handledmap.find(this) == handledmap.end())
  {
    handledmap[this] = true;        // the true is somewhat irrelevant, it is just used to store 'this' in the map

    // output header
    Printf(str, "%s%s ID '%s' name '%s':%s",
           CreateIndent(indent, ind_level).c_str(),
           GetType().c_str(), GetID().c_str(), GetName().c_str(),
           eol.c_str());

    // output known variables
    Dump(str, indent, eol, ind_level, "typeLabel", typeLabel);
    Dump(str, indent, eol, ind_level, "typeDefinition", typeDefinition);

    // call derived function to output variables from derived classes and recurse through references
    DumpEx(handledmap, str, indent, eol, ind_level);
  }
  else
  {
    // output information and note
    Printf(str, "%s%s ID '%s' name '%s' (see above)%s",
           CreateIndent(indent, ind_level).c_str(),
           GetType().c_str(), GetID().c_str(), GetName().c_str(),
           eol.c_str());
  }
}

/*--------------------------------------------------------------------------------*/
/** Generate XML for this and references objects in accordance with Tech3364
 *
 * @param str string to be modified
 * @param indent a string representing one level of indentation (e.g. a tab or spaces)
 * @param eol a string representing the end of line string
 * @param ind_level the initial indentation level
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::GenerateXML(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const
{
  std::vector<const ADMObject *>   reflist;
  std::map<const ADMObject *,bool> map;    // output each object only once
  uint_t i;

  // output list of references to objects and THEN output those objects themselves (which may output more references, etc.)
  XMLData(str, indent, eol, ind_level, reflist);

  // output information about objects stored in reflist
  for (i = 0; i < reflist.size(); i++)
  {
    const ADMObject *obj = reflist[i];

    // if object has not be outputted before, output it here
    if (map.find(obj) == map.end())
    {
      map[obj] = true;

      obj->XMLData(str, indent, eol, ind_level, reflist);
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Individual variable type XML attribute helper function
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::XMLAttribute(std::string& str, const std::string& name, const std::string& value)
{
  if (value != "") Printf(str, " %s=\"%s\"", name.c_str(), value.c_str());
}

/*--------------------------------------------------------------------------------*/
/** Individual variable type XML attribute helper function
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::XMLAttribute(std::string& str, const std::string& name, uint_t value)
{
  if (value != 0) Printf(str, " %s=\"%u\"", name.c_str(), value);
}

/*--------------------------------------------------------------------------------*/
/** Individual variable type XML attribute helper function
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::XMLAttribute(std::string& str, const std::string& name, double value)
{
  if (value != 0.0) Printf(str, " %s=\"%0.6f\"", name.c_str(), value);
}

/*--------------------------------------------------------------------------------*/
/** Individual variable type XML attribute helper function
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::XMLAttributeTime(std::string& str, const std::string& name, uint64_t value)
{
  if (value != 0) Printf(str, " %s=\"%s\"", name.c_str(), GenTime(value).c_str());
}

/*--------------------------------------------------------------------------------*/
/** 'Open' (start) an XML section
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::XMLOpen(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const
{
  // if this type is NOT an AudioBlockFormat object, output a blank line
  if (GetType() != ADMAudioBlockFormat::Type) Printf(str, "%s", eol.c_str());

  // output header
  Printf(str, "%s<%s",
         CreateIndent(indent, ind_level).c_str(),
         GetType().c_str());

  // AudioTrack objects are specified slightly differently
  if (GetType() == ADMAudioTrack::Type)
  {
    Printf(str, " UID=\"%s\"", GetID().c_str());
  }
  else
  {
    Printf(str, " %sID=\"%s\"", GetType().c_str(), GetID().c_str());
  }

  // output name only if it is valid
  if (GetName() != "") Printf(str, " %sName=\"%s\"",  GetType().c_str(), GetName().c_str());

  // output XML attributes
  XMLAttributes(str);

  // if no further data is needed, close the XML section now
  if (XMLEmpty()) Printf(str, " />%s", eol.c_str());
  else            Printf(str, ">%s", eol.c_str());
}

/*--------------------------------------------------------------------------------*/
/** Close an XML section
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::XMLClose(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const
{
  // only need to close XML section here if some XML data has been outputted
  if (!XMLEmpty())
  {
    Printf(str, "%s</%s>%s",
           CreateIndent(indent, ind_level).c_str(),
           GetType().c_str(),
           eol.c_str());
  }
}

/*--------------------------------------------------------------------------------*/
/** Output XML reference information for this object
 *
 * @note there is no need to call this function directly, it is called as part of GenerateXML()
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::XMLRef(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const
{
  std::string ref = GetReference();

  Printf(str, "%s<%s>%s</%s>%s",
         CreateIndent(indent, ind_level).c_str(),
         ref.c_str(),
         GetID().c_str(),
         ref.c_str(),
         eol.c_str());
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
/** Dump additional information about this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioProgramme::DumpEx(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const
{
  uint_t i;

  Dump(str, indent, eol, ind_level, "language", language);

  for (i = 0; i < contentrefs.size(); i++)
  {
    contentrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
  }
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML attributes for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioProgramme::XMLAttributes(std::string& str) const
{
  ADMObject::XMLAttributes(str);

  XMLAttribute(str, "language", language);
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML data for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioProgramme::XMLData(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, std::vector<const ADMObject *>& reflist) const
{
  uint_t i;

  XMLOpen(str, indent, eol, ind_level);

  ADMObject::XMLData(str, indent, eol, ind_level, reflist);
        
  for (i = 0; i < contentrefs.size(); i++)
  {
    contentrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
    reflist.push_back(contentrefs[i]);
  }

  XMLClose(str, indent, eol, ind_level);
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
/** Dump additional information about this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioContent::DumpEx(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const
{
  uint_t i;

  Dump(str, indent, eol, ind_level, "language", language);

  for (i = 0; i < objectrefs.size(); i++)
  {
    objectrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
  }
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML attributes for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioContent::XMLAttributes(std::string& str) const
{
  ADMObject::XMLAttributes(str);

  XMLAttribute(str, "language", language);
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML data for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioContent::XMLData(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, std::vector<const ADMObject *>& reflist) const
{
  uint_t i;

  XMLOpen(str, indent, eol, ind_level);
        
  ADMObject::XMLData(str, indent, eol, ind_level, reflist);

  for (i = 0; i < objectrefs.size(); i++)
  {
    objectrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
    reflist.push_back(objectrefs[i]);
  }

  XMLClose(str, indent, eol, ind_level);
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
/** Dump additional information about this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioObject::DumpEx(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const
{
  uint_t i;

  DumpTime(str, indent, eol, ind_level, "startTime", startTime);
  DumpTime(str, indent, eol, ind_level, "duration", duration);

  Dump(str, indent, eol, ind_level,     "block channel",   GetChildrenStartChannel());
  Dump(str, indent, eol, ind_level,     "block nchannels", GetChildrenChannelCount());

  DumpTime(str, indent, eol, ind_level, "block start",     GetChildrenStartTime());
  DumpTime(str, indent, eol, ind_level, "block end",       GetChildrenEndTime());

  for (i = 0; i < objectrefs.size(); i++)
  {
    objectrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
  }

  for (i = 0; i < packformatrefs.size(); i++)
  {
    packformatrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
  }

  for (i = 0; i < trackrefs.size(); i++)
  {
    trackrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
  }
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML attributes for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioObject::XMLAttributes(std::string& str) const
{
  ADMObject::XMLAttributes(str);

  XMLAttributeTime(str, "startTime", startTime);
  XMLAttributeTime(str, "duration", duration);
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML data for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioObject::XMLData(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, std::vector<const ADMObject *>& reflist) const
{
  uint_t i;

  XMLOpen(str, indent, eol, ind_level);
        
  ADMObject::XMLData(str, indent, eol, ind_level, reflist);

  for (i = 0; i < objectrefs.size(); i++)
  {
    objectrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
    reflist.push_back(objectrefs[i]);
  }

  for (i = 0; i < packformatrefs.size(); i++)
  {
    packformatrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
    reflist.push_back(packformatrefs[i]);
  }

  for (i = 0; i < trackrefs.size(); i++)
  {
    trackrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
    reflist.push_back(trackrefs[i]);
  }

  XMLClose(str, indent, eol, ind_level);
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
/** Dump additional information about this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrack::DumpEx(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const
{
  uint_t i;

  Dump(str, indent, eol, ind_level, "trackNum",   trackNum);
  Dump(str, indent, eol, ind_level, "sampleRate", sampleRate);
  Dump(str, indent, eol, ind_level, "bitDepth",   bitDepth);

  for (i = 0; i < trackformatrefs.size(); i++)
  {
    trackformatrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
  }

  for (i = 0; i < packformatrefs.size(); i++)
  {
    packformatrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
  }
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML attributes for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrack::XMLAttributes(std::string& str) const
{
  ADMObject::XMLAttributes(str);

  XMLAttribute(str, "sampleRate", sampleRate);
  XMLAttribute(str, "bitDepth", bitDepth);
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML data for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrack::XMLData(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, std::vector<const ADMObject *>& reflist) const
{
  uint_t i;

  XMLOpen(str, indent, eol, ind_level);
        
  ADMObject::XMLData(str, indent, eol, ind_level, reflist);

  for (i = 0; i < trackformatrefs.size(); i++)
  {
    //trackformatrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
    reflist.push_back(trackformatrefs[i]);
  }

  for (i = 0; i < packformatrefs.size(); i++)
  {
    //packformatrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
    reflist.push_back(packformatrefs[i]);
  }

  XMLClose(str, indent, eol, ind_level);
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
/** Dump additional information about this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioPackFormat::DumpEx(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const
{
  uint_t i;

  DumpTime(str, indent, eol, ind_level, "start", GetChildrenStartTime());
  DumpTime(str, indent, eol, ind_level, "end",   GetChildrenEndTime());

  for (i = 0; i < channelformatrefs.size(); i++)
  {
    channelformatrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
  }

  for (i = 0; i < packformatrefs.size(); i++)
  {
    packformatrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
  }
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML attributes for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioPackFormat::XMLAttributes(std::string& str) const
{
  ADMObject::XMLAttributes(str);
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML data for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioPackFormat::XMLData(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, std::vector<const ADMObject *>& reflist) const
{
  uint_t i;

  XMLOpen(str, indent, eol, ind_level);

  ADMObject::XMLData(str, indent, eol, ind_level, reflist);

  for (i = 0; i < channelformatrefs.size(); i++)
  {
    channelformatrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
    reflist.push_back(channelformatrefs[i]);
  }

  for (i = 0; i < packformatrefs.size(); i++)
  {
    packformatrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
    reflist.push_back(packformatrefs[i]);
  }
    
  XMLClose(str, indent, eol, ind_level);
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
/** Dump additional information about this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::DumpEx(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const
{
  uint_t i;

  DumpTime(str, indent, eol, ind_level, "start", GetChildrenStartTime());
  DumpTime(str, indent, eol, ind_level, "end",   GetChildrenEndTime());

  Dump(str, indent, eol, ind_level, "formatLabel", formatLabel);
  Dump(str, indent, eol, ind_level, "formatDefinition", formatDefinition);

  for (i = 0; i < channelformatrefs.size(); i++)
  {
    channelformatrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
  }

  for (i = 0; i < packformatrefs.size(); i++)
  {
    packformatrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
  }

  for (i = 0; i < trackformatrefs.size(); i++)
  {
    trackformatrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
  }
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML attributes for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::XMLAttributes(std::string& str) const
{
  ADMObject::XMLAttributes(str);

  XMLAttribute(str, "formatLabel",      formatLabel);
  XMLAttribute(str, "formatDefinition", formatDefinition);
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML data for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::XMLData(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, std::vector<const ADMObject *>& reflist) const
{
  uint_t i;

  XMLOpen(str, indent, eol, ind_level);
        
  ADMObject::XMLData(str, indent, eol, ind_level, reflist);

  for (i = 0; i < channelformatrefs.size(); i++)
  {
    channelformatrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
    reflist.push_back(channelformatrefs[i]);
  }

  for (i = 0; i < packformatrefs.size(); i++)
  {
    packformatrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
    reflist.push_back(packformatrefs[i]);
  }

  XMLClose(str, indent, eol, ind_level);
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
/** Dump additional information about this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrackFormat::DumpEx(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const
{
  uint_t i;

  Dump(str, indent, eol, ind_level, "formatLabel", formatLabel);
  Dump(str, indent, eol, ind_level, "formatDefinition", formatDefinition);

  for (i = 0; i < streamformatrefs.size(); i++)
  {
    streamformatrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
  }
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML attributes for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrackFormat::XMLAttributes(std::string& str) const
{
  ADMObject::XMLAttributes(str);

  XMLAttribute(str, "formatLabel",      formatLabel);
  XMLAttribute(str, "formatDefinition", formatDefinition);
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML data for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrackFormat::XMLData(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, std::vector<const ADMObject *>& reflist) const
{
  uint_t i;

  XMLOpen(str, indent, eol, ind_level);
        
  ADMObject::XMLData(str, indent, eol, ind_level, reflist);

  for (i = 0; i < streamformatrefs.size(); i++)
  {
    streamformatrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
    reflist.push_back(streamformatrefs[i]);
  }

  XMLClose(str, indent, eol, ind_level);
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
/** Dump additional information about this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioChannelFormat::DumpEx(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const
{
  uint_t i;

  DumpTime(str, indent, eol, ind_level, "start", GetChildrenStartTime());
  DumpTime(str, indent, eol, ind_level, "end",   GetChildrenEndTime());

  for (i = 0; i < blockformatrefs.size(); i++)
  {
    blockformatrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
  }
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML attributes for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioChannelFormat::XMLAttributes(std::string& str) const
{
  ADMObject::XMLAttributes(str);
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML data for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioChannelFormat::XMLData(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, std::vector<const ADMObject *>& reflist) const
{
  uint_t i;

  XMLOpen(str, indent, eol, ind_level);

  ADMObject::XMLData(str, indent, eol, ind_level, reflist);

  for (i = 0; i < blockformatrefs.size(); i++)
  {
    if (i) Printf(str, "%s", eol.c_str());
    blockformatrefs[i]->XMLData(str, indent, eol, ind_level + 1, reflist);
  }
        
  XMLClose(str, indent, eol, ind_level);
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
/** Dump additional information about this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioBlockFormat::DumpEx(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const
{
  UNUSED_PARAMETER(handledmap);

  DumpTime(str, indent, eol, ind_level, "rtime", rtime);
  DumpTime(str, indent, eol, ind_level, "duration", duration);

  Printf(str, "%sposition %s %s%s", CreateIndent(indent, ind_level + 1).c_str(), position.ToString().c_str(), supplement.ToString().c_str(), eol.c_str());
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML attributes for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioBlockFormat::XMLAttributes(std::string& str) const
{
  ADMObject::XMLAttributes(str);

  XMLAttributeTime(str, "rtime", rtime);
  XMLAttributeTime(str, "duration", duration);
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML data for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioBlockFormat::XMLData(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, std::vector<const ADMObject *>& reflist) const
{
  XMLOpen(str, indent, eol, ind_level);

  ADMObject::XMLData(str, indent, eol, ind_level, reflist);

  if (position.polar)
  {
    Printf(str, "%s<position coordinate=\"azimuth\">%0.6f</position>%s",
           CreateIndent(indent, ind_level + 1).c_str(),
           position.pos.az,
           eol.c_str());
    Printf(str, "%s<position coordinate=\"elevation\">%0.6f</position>%s",
           CreateIndent(indent, ind_level + 1).c_str(),
           position.pos.el,
           eol.c_str());
    Printf(str, "%s<position coordinate=\"distance\">%0.6f</position>%s",
           CreateIndent(indent, ind_level + 1).c_str(),
           position.pos.d,
           eol.c_str());
  }
  else
  {
    Printf(str, "%s<position coordinate=\"x\">%0.6f</position>%s",
           CreateIndent(indent, ind_level + 1).c_str(),
           position.pos.x,
           eol.c_str());
    Printf(str, "%s<position coordinate=\"y\">%0.6f</position>%s",
           CreateIndent(indent, ind_level + 1).c_str(),
           position.pos.y,
           eol.c_str());
    Printf(str, "%s<position coordinate=\"z\">%0.6f</position>%s",
           CreateIndent(indent, ind_level + 1).c_str(),
           position.pos.z,
           eol.c_str());
  }
  bool diffuse;
  if (supplement.Get("diffuse", diffuse))
  {
    Printf(str, "%s<diffuse>%s</diffuse>%s",
           CreateIndent(indent, ind_level + 1).c_str(),
           diffuse ? "true" : "false",
           eol.c_str());
  }

  XMLClose(str, indent, eol, ind_level);
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
