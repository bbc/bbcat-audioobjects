
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include <map>
#include <algorithm>

#define DEBUG_LEVEL 2
#include <aplibs-dsp/ByteSwap.h>

#include "ADMObjects.h"
#include "ADMData.h"
#include "mt19937ar.h"
#include "crc32.h"

#define SERIALIZE_AS_TEXT 0

using namespace std;

BBC_AUDIOTOOLBOX_START

/*----------------------------------------------------------------------------------------------------*/

#if SERIALIZE_AS_TEXT
static struct {
	ADMObject::SerialDataType_t type;
	const char *name;
} _SerialDataTypes[] = {
	{ADMObject::SerialDataType_32bit, "32bit"},
	{ADMObject::SerialDataType_64bit, "64bit"},
	{ADMObject::SerialDataType_double, "double"},

	{ADMObject::SerialDataType_Time_ns, "Time_ns"},
	{ADMObject::SerialDataType_String, "String"},
	{ADMObject::SerialDataType_Position, "Position"},
	{ADMObject::SerialDataType_Position_Supplement, "Position_Supplement"},
	{ADMObject::SerialDataType_Reference, "Reference"},

	{ADMObject::SerialDataType_Values_And_Attributes, "Values_And_Attributes"},
	{ADMObject::SerialDataType_Attribute, "Attribute"},
	{ADMObject::SerialDataType_Value, "Value"},
	{ADMObject::SerialDataType_Value_Attributes, "Value_Attributes"},
	{ADMObject::SerialDataType_Value_Attribute, "Value_Attribute"},

	{ADMObject::SerialDataType_ADMHeader, "ADMHeader"},
	{ADMObject::SerialDataType_ObjectCRC, "ObjectCRC"},
	{ADMObject::SerialDataType_Programme, "Programme"},
	{ADMObject::SerialDataType_Content, "Content"},
	{ADMObject::SerialDataType_Object, "Object"},
	{ADMObject::SerialDataType_TrackUID, "TrackUID"},
	{ADMObject::SerialDataType_PackFormat, "PackFormat"},
	{ADMObject::SerialDataType_StreamFormat, "StreamFormat"},
	{ADMObject::SerialDataType_ChannelFormat, "ChannelFormat"},
	{ADMObject::SerialDataType_BlockFormat, "BlockFormat"},
	{ADMObject::SerialDataType_TrackFormat, "TrackFormat"},
};

map<ADMObject::SerialDataType_t,const char *> SerialDataType_Names;

#endif

/*--------------------------------------------------------------------------------*/
/** Base constructor for all objects
 *
 * @param _owner an instance of ADMData that this object should belong to
 * @param _id unique ID for this object (specified as part of the ADM)
 * @param _name optional human-friendly name of the object
 *
 */
/*--------------------------------------------------------------------------------*/
ADMObject::ADMObject(ADMData& _owner, const string& _id, const string& _name) : owner(_owner),
																				id(_id),
																				name(_name)
{
#if SERIALIZE_AS_TEXT
	if (SerialDataType_Names.size() == 0) {
		uint_t i;

		for (i = 0; i < NUMBEROF(_SerialDataTypes); i++) {
			SerialDataType_Names[_SerialDataTypes[i].type] = _SerialDataTypes[i].name;
		}
	}
#endif
}

/*--------------------------------------------------------------------------------*/
/** Register this object with the owner
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::Register()
{
	owner.Register(this, GetType());
}

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetValues()
{
	SetValue(typeLabel, "typeLabel");
}

/*--------------------------------------------------------------------------------*/
/** Try to connect references after all objects have been set up
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SetReferences()
{
	ADMVALUES::iterator it;

	// cycle through values looking for references to the specified object type
	for (it = values.begin(); (it != values.end());) {
		const ADMObject *obj  = NULL; // for neater response handling
		const ADMVALUE& value = *it;
		bool  refrejected = false;

		// the value name is reference name of object type
		if (value.name == ADMAudioContent::Reference) {
			ADMAudioContent *ref;

			// look up reference using owner ADMData object, try to cast it to an object of the correct type
			if ((ref = dynamic_cast<ADMAudioContent *>(owner.GetReference(value))) != NULL) {
				// save object for debugging purposes
				obj = ref;

				// store object reference
				refrejected = !Add(ref);
			}
		}
		else if (value.name == ADMAudioObject::Reference) {
			ADMAudioObject *ref;

			if ((ref = dynamic_cast<ADMAudioObject *>(owner.GetReference(value))) != NULL) {
				// save object for debugging purposes
				obj = ref;

				// store object reference
				refrejected = !Add(ref);
			}
		}
		else if (value.name == ADMAudioTrack::Reference) {
			ADMAudioTrack *ref;

			if ((ref = dynamic_cast<ADMAudioTrack *>(owner.GetReference(value))) != NULL) {
				// save object for debugging purposes
				obj = ref;

				// store object reference
				refrejected = !Add(ref);
			}
		}
		else if (value.name == ADMAudioPackFormat::Reference) {
			ADMAudioPackFormat *ref;

			if ((ref = dynamic_cast<ADMAudioPackFormat *>(owner.GetReference(value))) != NULL) {
				// save object for debugging purposes
				obj = ref;

				// store object reference
				refrejected = !Add(ref);
			}
		}
		else if (value.name == ADMAudioStreamFormat::Reference) {
			ADMAudioStreamFormat *ref;

			if ((ref = dynamic_cast<ADMAudioStreamFormat *>(owner.GetReference(value))) != NULL) {
				// save object for debugging purposes
				obj = ref;

				// store object reference
				refrejected = !Add(ref);
			}
		}
		else if (value.name == ADMAudioTrackFormat::Reference) {
			ADMAudioTrackFormat *ref;

			if ((ref = dynamic_cast<ADMAudioTrackFormat *>(owner.GetReference(value))) != NULL) {
				// save object for debugging purposes
				obj = ref;

				// store object reference
				refrejected = !Add(ref);
			}
		}
		else if (value.name == ADMAudioChannelFormat::Reference) {
			ADMAudioChannelFormat *ref;

			if ((ref = dynamic_cast<ADMAudioChannelFormat *>(owner.GetReference(value))) != NULL) {
				// save object for debugging purposes
				obj = ref;

				// store object reference
				refrejected = !Add(ref);
			}
		}
		else if (value.name == ADMAudioBlockFormat::Reference) {
			ADMAudioBlockFormat *ref;

			if ((ref = dynamic_cast<ADMAudioBlockFormat *>(owner.GetReference(value))) != NULL) {
				// save object for debugging purposes
				obj = ref;

				// store object reference
				refrejected = !Add(ref);
			}
		}
		else {
			++it;
			// note continue to avoid removing non-reference values
			continue;
		}

		if (obj) {
			if (refrejected) {
				ERROR("Reference %s as reference '%s' for %s REJECTED",
					  obj->ToString().c_str(),
					  value.value.c_str(),
					  ToString().c_str());
			}
			else {
				DEBUG3(("Found %s as reference '%s' for %s",
						obj->ToString().c_str(),
						value.value.c_str(),
						ToString().c_str()));
			}
		}
		else {
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
void ADMObject::XMLAttributes(string& str) const
{
	ADMVALUES::const_iterator it;

	// typeLabel can appear in most objects, handle it here
	XMLAttribute(str, "typeLabel", typeLabel);

	// output XML attribute values from list - this allows arbitary attributes to be stored
	for (it = values.begin(); it != values.end(); ++it) {
		const ADMVALUE& value = *it;

		if (value.attr) XMLAttribute(str, value.name, value.value);
	}
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML data for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::XMLData(string& str, const string& indent, const string& eol, uint_t ind_level, vector<const ADMObject *>& reflist) const
{
	ADMVALUES::const_iterator it;

	UNUSED_PARAMETER(reflist);

	// this base object version will be called by every derived version

	// cycle through all values looking for non-attribute values and output them
	for (it = values.begin(); it != values.end(); ++it) {
		const ADMVALUE& value = *it;

		if (!value.attr) {
			ADMATTRS::const_iterator it2;

			// create new XML section for this value
			Printf(str, "%s<%s", CreateIndent(indent, ind_level + 1).c_str(), value.name.c_str());

			// add sub-attributes as XML attributes of this section
			for (it2 = value.attrs.begin(); it2 != value.attrs.end(); ++it2) {
				XMLAttribute(str, it2->first.c_str(), it2->second.c_str());
			}

			// if value is valid, output it and close the XML section
			if (value.value != "") {
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
const ADMObject::ADMVALUE* ADMObject::GetValue(const string& name) const
{
	const ADMVALUE *value = NULL;
	uint_t i;

	// simple search and compare
	// (MUST use a list because there can be MULTIPLE values of the same name)
	for (i = 0; i < values.size(); i++) {
		if (values[i].name == name) {
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
const string *ADMObject::GetAttribute(const ADMVALUE& value, const string& name) const
{
	ADMATTRS::const_iterator it;
	const string *attr = NULL;

	if ((it = value.attrs.find(name)) != value.attrs.end()) {
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
	
	for (it = values.begin(); it != values.end(); ++it) {
		const ADMVALUE& value1 = *it;

		if (value == &value1) {
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
bool ADMObject::SetValue(string& res, const string& name)
{
	const ADMVALUE *value;
	bool success = false;

	if ((value = GetValue(name)) != NULL) {
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
bool ADMObject::SetValue(double& res, const string& name)
{
	const ADMVALUE *value;
	bool success = false;

	if ((value = GetValue(name)) != NULL) {
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
bool ADMObject::SetValue(uint_t& res, const string& name)
{
	const ADMVALUE *value;
	bool success = false;

	if ((value = GetValue(name)) != NULL) {
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
bool ADMObject::SetValue(ulong_t& res, const string& name)
{
	const ADMVALUE *value;
	bool success = false;

	if ((value = GetValue(name)) != NULL) {
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
bool ADMObject::SetValue(sint_t& res, const string& name)
{
	const ADMVALUE *value;
	bool success = false;

	if ((value = GetValue(name)) != NULL) {
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
bool ADMObject::SetValue(slong_t& res, const string& name)
{
	const ADMVALUE *value;
	bool success = false;

	if ((value = GetValue(name)) != NULL) {
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
bool ADMObject::SetValue(bool& res, const string& name)
{
	const ADMVALUE *value;
	bool success = false;

	if ((value = GetValue(name)) != NULL) {
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
bool ADMObject::SetValueTime(uint64_t& res, const string& name)
{
	const ADMVALUE *value;
	bool success = false;

	if ((value = GetValue(name)) != NULL) {
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
bool ADMObject::CalcTime(uint64_t& t, const string& str)
{
	uint_t hr, mn, s, ss;
	bool   success = false;

	if (sscanf(str.c_str(), "%u:%u:%u.%u", &hr, &mn, &s, &ss) == 4) {
		t = hr;					// hours
		t = (t * 60) + mn;		// minutes
		t = (t * 60) + s;		// seconds
		t = (t * 100000) + ss;	// 100000ths of second
		t *= 10000;				// nanoseconds
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
string ADMObject::GenTime(uint64_t t)
{
	string str;
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
void ADMObject::Dump(string& str, const string& indent, const string& eol, uint_t ind_level, const string& name, const string& value)
{
	if (value != "") {
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
void ADMObject::Dump(string& str, const string& indent, const string& eol, uint_t ind_level, const string& name, uint_t value)
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
void ADMObject::Dump(string& str, const string& indent, const string& eol, uint_t ind_level, const string& name, bool value)
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
void ADMObject::Dump(string& str, const string& indent, const string& eol, uint_t ind_level, const string& name, double value)
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
void ADMObject::DumpTime(string& str, const string& indent, const string& eol, uint_t ind_level, const string& name, uint64_t value)
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
void ADMObject::Dump(string& str, const string& indent, const string& eol, uint_t ind_level) const
{
	map<const ADMObject *,bool> handledmap;		// ensures each object is output ONLY once since references can be circular

	Dump(handledmap, str, indent, eol, ind_level);
}

/*--------------------------------------------------------------------------------*/
/** A handler function for the above - DO NOT CALL THIS DIRECTLY, call the above
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::Dump(map<const ADMObject *,bool>& handledmap, string& str, const string& indent, const string& eol, uint_t ind_level) const
{
	// if this object has not been outputted previously
	if (handledmap.find(this) == handledmap.end()) {
		handledmap[this] = true;		// the true is somewhat irrelevant, it is just used to store 'this' in the map

		// output header
		Printf(str, "%s%s ID '%s' name '%s':%s",
			   CreateIndent(indent, ind_level).c_str(),
			   GetType().c_str(), GetID().c_str(), GetName().c_str(),
			   eol.c_str());

		// output known variables
		Dump(str, indent, eol, ind_level, "typeLabel", typeLabel);

		// call derived function to output variables from derived classes and recurse through references
		DumpEx(handledmap, str, indent, eol, ind_level);
	}
	else {
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
void ADMObject::GenerateXML(string& str, const string& indent, const string& eol, uint_t ind_level) const
{
	vector<const ADMObject *>   reflist;
	map<const ADMObject *,bool> map;	// output each object only once
	uint_t i;

	// output list of references to objects and THEN output those objects themselves (which may output more references, etc.)
	XMLData(str, indent, eol, ind_level, reflist);

	// output information about objects stored in reflist
	for (i = 0; i < reflist.size(); i++) {
		const ADMObject *obj = reflist[i];

		// if object has not be outputted before, output it here
		if (map.find(obj) == map.end()) {
			map[obj] = true;

			obj->XMLData(str, indent, eol, ind_level, reflist);
		}
	}
}

/*--------------------------------------------------------------------------------*/
/** Individual variable type XML attribute helper function
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::XMLAttribute(string& str, const string& name, const string& value)
{
	if (value != "") Printf(str, " %s=\"%s\"", name.c_str(), value.c_str());
}

/*--------------------------------------------------------------------------------*/
/** Individual variable type XML attribute helper function
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::XMLAttribute(string& str, const string& name, uint_t value)
{
	if (value != 0) Printf(str, " %s=\"%u\"", name.c_str(), value);
}

/*--------------------------------------------------------------------------------*/
/** Individual variable type XML attribute helper function
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::XMLAttribute(string& str, const string& name, double value)
{
	if (value != 0.0) Printf(str, " %s=\"%0.6f\"", name.c_str(), value);
}

/*--------------------------------------------------------------------------------*/
/** Individual variable type XML attribute helper function
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::XMLAttributeTime(string& str, const string& name, uint64_t value)
{
	if (value != 0) Printf(str, " %s=\"%s\"", name.c_str(), GenTime(value).c_str());
}

/*--------------------------------------------------------------------------------*/
/** 'Open' (start) an XML section
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::XMLOpen(string& str, const string& indent, const string& eol, uint_t ind_level) const
{
	// if this type is NOT an AudioBlockFormat object, output a blank line
	if (GetType() != ADMAudioBlockFormat::Type) Printf(str, "%s", eol.c_str());

	// output header
	Printf(str, "%s<%s",
		   CreateIndent(indent, ind_level).c_str(),
		   GetType().c_str());

	// AudioTrack objects are specified slightly differently
	if (GetType() == ADMAudioTrack::Type) {
		Printf(str, " UID=\"%s\"", GetID().c_str());
	}
	else {
		Printf(str, " %sID=\"%s\"", GetType().c_str(), GetID().c_str());
	}

	// output name only if it is valid
	if (GetName() != "") Printf(str, " %sName=\"%s\"",  GetType().c_str(), GetName().c_str());

	// output XML attributes
	XMLAttributes(str);

	// if no further data is needed, close the XML section now
	if (XMLEmpty()) Printf(str, " />%s", eol.c_str());
	else			Printf(str, ">%s", eol.c_str());
}

/*--------------------------------------------------------------------------------*/
/** Close an XML section
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::XMLClose(string& str, const string& indent, const string& eol, uint_t ind_level) const
{
	// only need to close XML section here if some XML data has been outputted
	if (!XMLEmpty()) {
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
void ADMObject::XMLRef(string& str, const string& indent, const string& eol, uint_t ind_level) const
{
	string ref = GetReference();

	Printf(str, "%s<%s>%s</%s>%s",
		   CreateIndent(indent, ind_level).c_str(),
		   ref.c_str(),
		   GetID().c_str(),
		   ref.c_str(),
		   eol.c_str());
}

/*--------------------------------------------------------------------------------*/
/** Serialize (or prepare to serialize) this object
 *
 * @param dst destination buffer or NULL to calculate required buffer size
 * @param len offset into data to store data, modified by function
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::Serialize(uint8_t *dst, uint_t& len) const
{
	uint_t len0   = len;
	uint_t sublen = 0;

	if (dst) Serialize(NULL, sublen);
	
	SerializeData(dst, len, GetSerialDataType(), sublen);
	SerializeSync(dst, len, len0);

	SerializeItem(dst, len, "id", id);
	SerializeItem(dst, len, "name", name);
	SerializeItem(dst, len, "typeLabel", typeLabel);
	SerializeData(dst, len, values);
	SerializeEx(dst, len);
	SerializeObjectCRC(dst, len, len0);
}

/*--------------------------------------------------------------------------------*/
/** Serialize a sync block to aid syncing (random 32-bit number followed by 32-bit CRC)
 *
 * @param dst destination buffer or NULL to calculate required buffer size
 * @param len offset into data to store data, modified by function
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SerializeSync(uint8_t *dst, uint_t& len, uint_t len0)
{
	uint32_t rval = 0, crc = 0;
	if (dst) rval = genrand_int32();
	
	SerializeData(dst, len, rval);
	if (dst) crc  = CRC32(dst + len0, len - len0);
	SerializeData(dst, len, crc);
}

/*--------------------------------------------------------------------------------*/
/** Add data to a serilization buffer (or calculate size of data)
 *
 * @param dst buffer to receive data (or NULL to just calculate size)
 * @param len offset into buffer, modified by this function
 * @param obj object to store
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMObject::SerializeData(uint8_t *dst, uint_t& len, const void *obj, uint_t objlen, bool byteswap)
{
	if (dst) {
		memcpy(dst + len, obj, objlen);
		if (byteswap) ByteSwap(dst + len, objlen);
	}
	len += objlen;
}

void ADMObject::SerializeData(uint8_t *dst, uint_t& len, uint8_t obj)
{
#if SERIALIZE_AS_TEXT
	string str;
	Printf(str, "8-bit %02lx\n", (ulong_t)obj);
	SerializeData(dst, len, str.c_str(), str.length());
#else
	SerializeData(dst, len, &obj, sizeof(obj));
#endif
}

void ADMObject::SerializeData(uint8_t *dst, uint_t& len, uint16_t obj)
{
#if SERIALIZE_AS_TEXT
	string str;
	Printf(str, "16-bit %04lx\n", (ulong_t)obj);
	SerializeData(dst, len, str.c_str(), str.length());
#else
	SerializeData(dst, len, &obj, sizeof(obj), MACHINE_IS_BIG_ENDIAN);
#endif
}

void ADMObject::SerializeData(uint8_t *dst, uint_t& len, uint32_t obj, uint_t bytes)
{
#if SERIALIZE_AS_TEXT
	string str;
	Printf(str, "32-bit %08lx (%u bytes)\n", (ulong_t)obj, bytes);
	SerializeData(dst, len, str.c_str(), str.length());
#else
	SerializeData(dst, len, (const uint8_t *)&obj + (MACHINE_IS_BIG_ENDIAN ? sizeof(obj) - bytes : 0), bytes, MACHINE_IS_BIG_ENDIAN);
#endif
}

void ADMObject::SerializeData(uint8_t *dst, uint_t& len, uint64_t obj, uint_t bytes)
{
#if SERIALIZE_AS_TEXT
	string str;
	Printf(str, "64-bit %016lx (%u bytes)\n", (ulong_t)obj, bytes);
	SerializeData(dst, len, str.c_str(), str.length());
#else
	SerializeData(dst, len, (const uint8_t *)&obj + (MACHINE_IS_BIG_ENDIAN ? sizeof(obj) - bytes : 0), bytes, MACHINE_IS_BIG_ENDIAN);
#endif
}

void ADMObject::SerializeData(uint8_t *dst, uint_t& len, double obj)
{
#if SERIALIZE_AS_TEXT
	string str;
	Printf(str, "double %24.16le\n", obj);
	SerializeData(dst, len, str.c_str(), str.length());
#else
	SerializeData(dst, len, &obj, sizeof(obj), MACHINE_IS_BIG_ENDIAN);
#endif
}

void ADMObject::SerializeData(uint8_t *dst, uint_t& len, const std::string& obj)
{
#if SERIALIZE_AS_TEXT
	string str;
	Printf(str, "String '%s'\n", obj.c_str());
	SerializeData(dst, len, str.c_str(), str.length());
#else
	uint16_t objlen = obj.length();
	SerializeData(dst, len, objlen);
	SerializeData(dst, len, obj.c_str(), objlen);
#endif
}

void ADMObject::SerializeData(uint8_t *dst, uint_t& len, SerialDataType_t type, uint_t sublen)
{
#if SERIALIZE_AS_TEXT
	string str;
	Printf(str, "Type %u (%s), length %-10u\n", (uint_t)type, SerialDataType_Names[type], (uint_t)(sublen - sizeof(uint32_t)));
	SerializeData(dst, len, str.c_str(), str.length());
#else
	SerializeData(dst, len, (uint8_t)type);
	SerializeData(dst, len, (uint32_t)(sublen - sizeof(uint32_t)), 3);
#endif
}

void ADMObject::SerializeData(uint8_t *dst, uint_t& len, SerialDataType_t type)
{
#if SERIALIZE_AS_TEXT
	string str;
	Printf(str, "Type %u (%s)\n", (uint_t)type, SerialDataType_Names[type]);
	SerializeData(dst, len, str.c_str(), str.length());
#else
	SerializeData(dst, len, (uint8_t)type);
#endif
}

void ADMObject::SerializeData(uint8_t *dst, uint_t& len, const ADMATTRS& obj)
{
	ADMATTRS::const_iterator it;
	uint_t sublen = 0;

	if (dst) SerializeData(NULL, sublen, obj);

	SerializeData(dst, len, SerialDataType_Value_Attributes, sublen);
	for (it = obj.begin(); it != obj.end(); ++it) {
		SerializeItem(dst, len, it->first, it->second);
	}
}

void ADMObject::SerializeData(uint8_t *dst, uint_t& len, const ADMVALUE& obj)
{
	SerializeItem(dst, len, obj.name, obj.value);
	if (!obj.attr) SerializeData(dst, len, obj.attrs);
}

void ADMObject::SerializeData(uint8_t *dst, uint_t& len, const ADMVALUES& obj)
{
	ADMVALUES::const_iterator it;
	uint_t sublen = 0;

	if (dst) SerializeData(NULL, sublen, obj);

	SerializeData(dst, len, SerialDataType_Values_And_Attributes, sublen);

	// cycle through values serializing each item
	for (it = obj.begin(); it != obj.end(); ++it) {
		SerializeData(dst, len, *it);
	}
}

void ADMObject::SerializeData(uint8_t *dst, uint_t& len, const ADMObject *obj)
{
	uint_t sublen = 0;

	if (dst) SerializeData(NULL, sublen, obj);

	SerializeData(dst, len, SerialDataType_Reference, sublen);
	SerializeData(dst, len, obj->GetSerialDataType());
	SerializeData(dst, len, obj->GetID());
}

void ADMObject::SerializeData(uint8_t *dst, uint_t& len, const Position& obj)
{
	SerializeData(dst, len, obj.polar);
	if (obj.polar) {
		SerializeData(dst, len, obj.pos.x);
		SerializeData(dst, len, obj.pos.y);
		SerializeData(dst, len, obj.pos.z);
	}
	else {
		SerializeData(dst, len, obj.pos.az);
		SerializeData(dst, len, obj.pos.el);
		SerializeData(dst, len, obj.pos.d);
	}
}

void ADMObject::SerializeData(uint8_t *dst, uint_t& len, const ParameterSet& obj)
{
	ParameterSet::Iterator it;
	uint_t sublen = 0;

	if (dst) SerializeData(NULL, sublen, obj);

	SerializeData(dst, len, SerialDataType_Position_Supplement, sublen);

	for (it = obj.GetBegin(); it != obj.GetEnd(); ++it) {
		SerializeData(dst, len, it->first);
		SerializeData(dst, len, it->second);
	}
}

void ADMObject::SerializeItem(uint8_t *dst, uint_t& len, const std::string& name, uint32_t obj)
{
	uint_t sublen = 0;

	if (dst) SerializeItem(NULL, sublen, name, obj);

	SerializeData(dst, len, SerialDataType_32bit, sublen);
	SerializeData(dst, len, name);
	SerializeData(dst, len, obj);
}

void ADMObject::SerializeItem(uint8_t *dst, uint_t& len, const std::string& name, uint64_t obj)
{
	uint_t sublen = 0;

	if (dst) SerializeItem(NULL, sublen, name, obj);

	SerializeData(dst, len, SerialDataType_64bit, sublen);
	SerializeData(dst, len, name);
	SerializeData(dst, len, obj);
}

void ADMObject::SerializeItem(uint8_t *dst, uint_t& len, const std::string& name, double obj)
{
	uint_t sublen = 0;

	if (dst) SerializeItem(NULL, sublen, name, obj);

	SerializeData(dst, len, SerialDataType_double, sublen);
	SerializeData(dst, len, name);
	SerializeData(dst, len, obj);
}

void ADMObject::SerializeTime(uint8_t *dst, uint_t& len, const std::string& name, uint64_t obj)
{
	uint_t sublen = 0;

	if (dst) SerializeTime(NULL, sublen, name, obj);

	SerializeData(dst, len, SerialDataType_Time_ns, sublen);
	SerializeData(dst, len, name);
	SerializeData(dst, len, obj);
}

void ADMObject::SerializeItem(uint8_t *dst, uint_t& len, const std::string& name, const std::string& obj)
{
	uint_t sublen = 0;

	if (dst) SerializeItem(NULL, sublen, name, obj);

	SerializeData(dst, len, SerialDataType_String, sublen);
	SerializeData(dst, len, name);
	SerializeData(dst, len, obj);
}

void ADMObject::SerializeItem(uint8_t *dst, uint_t& len, const std::string& name, const Position& obj, const ParameterSet& obj2)
{
	uint_t sublen = 0;

	if (dst) SerializeItem(NULL, sublen, name, obj, obj2);

	SerializeData(dst, len, SerialDataType_Position, sublen);
	SerializeData(dst, len, name);
	SerializeData(dst, len, obj);
	SerializeData(dst, len, obj2);
}

void ADMObject::SerializeObjectCRC(uint8_t *dst, uint_t& len, uint_t len0)
{
	uint32_t crc = 0;

	SerializeData(dst, len, SerialDataType_ObjectCRC, 2 * sizeof(uint32_t));
	if (dst) crc  = CRC32(dst + len0, len - len0);
	SerializeData(dst, len, crc);

#if SERIALIZE_AS_TEXT
	{
		static const char *tail = "\n";
		static const int  tlen  = strlen(tail);
		if (dst) memcpy(dst + len, tail, tlen);
		len += tlen;
	}
#endif
}

/*----------------------------------------------------------------------------------------------------*/

const string ADMAudioProgramme::Type      = "audioProgramme";
const string ADMAudioProgramme::Reference = Type + "IDRef";

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
	if (std::find(contentrefs.begin(), contentrefs.end(), obj) == contentrefs.end()) {
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
void ADMAudioProgramme::DumpEx(map<const ADMObject *,bool>& handledmap, string& str, const string& indent, const string& eol, uint_t ind_level) const
{
	uint_t i;

	Dump(str, indent, eol, ind_level, "language", language);

	for (i = 0; i < contentrefs.size(); i++) {
		contentrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
	}
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML attributes for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioProgramme::XMLAttributes(string& str) const
{
	ADMObject::XMLAttributes(str);

	XMLAttribute(str, "language", language);
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML data for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioProgramme::XMLData(string& str, const string& indent, const string& eol, uint_t ind_level, vector<const ADMObject *>& reflist) const
{
	uint_t i;

	XMLOpen(str, indent, eol, ind_level);

	ADMObject::XMLData(str, indent, eol, ind_level, reflist);
		
	for (i = 0; i < contentrefs.size(); i++) {
		contentrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
		reflist.push_back(contentrefs[i]);
	}

	XMLClose(str, indent, eol, ind_level);
}

/*--------------------------------------------------------------------------------*/
/** Serialize (or prepare to serialize) additional parts of this object
 *
 * @param dst destination buffer or NULL to calculate required buffer size
 * @param len offset into data to store data, modified by function
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioProgramme::SerializeEx(uint8_t *dst, uint_t& len) const
{
	uint_t i;

	SerializeItem(dst, len, "language", language);

	for (i = 0; i < contentrefs.size(); i++) {
		SerializeData(dst, len, contentrefs[i]);
	}
}

/*----------------------------------------------------------------------------------------------------*/

const string ADMAudioContent::Type      = "audioContent";
const string ADMAudioContent::Reference = Type + "IDRef";

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
	if (std::find(objectrefs.begin(), objectrefs.end(), obj) == objectrefs.end()) {
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
void ADMAudioContent::DumpEx(map<const ADMObject *,bool>& handledmap, string& str, const string& indent, const string& eol, uint_t ind_level) const
{
	uint_t i;

	Dump(str, indent, eol, ind_level, "language", language);

	for (i = 0; i < objectrefs.size(); i++) {
		objectrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
	}
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML attributes for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioContent::XMLAttributes(string& str) const
{
	ADMObject::XMLAttributes(str);

	XMLAttribute(str, "language", language);
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML data for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioContent::XMLData(string& str, const string& indent, const string& eol, uint_t ind_level, vector<const ADMObject *>& reflist) const
{
	uint_t i;

	XMLOpen(str, indent, eol, ind_level);
		
	ADMObject::XMLData(str, indent, eol, ind_level, reflist);

	for (i = 0; i < objectrefs.size(); i++) {
		objectrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
		reflist.push_back(objectrefs[i]);
	}

	XMLClose(str, indent, eol, ind_level);
}

/*--------------------------------------------------------------------------------*/
/** Serialize (or prepare to serialize) additional parts of this object
 *
 * @param dst destination buffer or NULL to calculate required buffer size
 * @param len offset into data to store data, modified by function
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioContent::SerializeEx(uint8_t *dst, uint_t& len) const
{
	uint_t i;

	SerializeItem(dst, len, "language", language);

	for (i = 0; i < objectrefs.size(); i++) {
		SerializeData(dst, len, objectrefs[i]);
	}
}

/*----------------------------------------------------------------------------------------------------*/

const string ADMAudioObject::Type      = "audioObject";
const string ADMAudioObject::Reference = Type + "IDRef";

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
ADMAudioObject::ADMAudioObject(ADMData& _owner, const std::string& _id, const std::string& _name) : ADMObject(_owner, _id, _name),
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
	if (std::find(objectrefs.begin(), objectrefs.end(), obj) == objectrefs.end()) {
		objectrefs.push_back(obj);
		return true;
	}

	// reference is already in the list
	return true;
}

bool ADMAudioObject::Add(ADMAudioPackFormat *obj)
{
	if (std::find(packformatrefs.begin(), packformatrefs.end(), obj) == packformatrefs.end()) {
		packformatrefs.push_back(obj);
		return true;
	}

	// reference is already in the list
	return true;
}

bool ADMAudioObject::Add(ADMAudioTrack *obj)
{
	if (std::find(trackrefs.begin(), trackrefs.end(), obj) == trackrefs.end()) {
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

	for (i = 0; i < trackrefs.size(); i++) {
		uint_t t = trackrefs[i]->GetTrackNum() - 1;
		childrenMinChannel = MIN(childrenMinChannel, t);
		childrenMaxChannel = MAX(childrenMaxChannel, t);
	}

	for (i = 0; i < packformatrefs.size(); i++) {
		ADMAudioPackFormat *obj = packformatrefs[i];

		obj->UpdateLimits();
		Update(obj);
	}

	for (i = 0; i < trackrefs.size(); i++) {
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
void ADMAudioObject::DumpEx(map<const ADMObject *,bool>& handledmap, string& str, const string& indent, const string& eol, uint_t ind_level) const
{
	uint_t i;

	DumpTime(str, indent, eol, ind_level, "startTime", startTime);
	DumpTime(str, indent, eol, ind_level, "duration", duration);

	Dump(str, indent, eol, ind_level, 	  "block channel",   GetChildrenStartChannel());
	Dump(str, indent, eol, ind_level, 	  "block nchannels", GetChildrenChannelCount());

	DumpTime(str, indent, eol, ind_level, "block start", 	 GetChildrenStartTime());
	DumpTime(str, indent, eol, ind_level, "block end",   	 GetChildrenEndTime());

	for (i = 0; i < objectrefs.size(); i++) {
		objectrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
	}

	for (i = 0; i < packformatrefs.size(); i++) {
		packformatrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
	}

	for (i = 0; i < trackrefs.size(); i++) {
		trackrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
	}
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML attributes for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioObject::XMLAttributes(string& str) const
{
	ADMObject::XMLAttributes(str);

	XMLAttributeTime(str, "startTime", startTime);
	XMLAttributeTime(str, "duration", duration);
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML data for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioObject::XMLData(string& str, const string& indent, const string& eol, uint_t ind_level, vector<const ADMObject *>& reflist) const
{
	uint_t i;

	XMLOpen(str, indent, eol, ind_level);
		
	ADMObject::XMLData(str, indent, eol, ind_level, reflist);

	for (i = 0; i < objectrefs.size(); i++) {
		objectrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
		reflist.push_back(objectrefs[i]);
	}

	for (i = 0; i < packformatrefs.size(); i++) {
		packformatrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
		reflist.push_back(packformatrefs[i]);
	}

	for (i = 0; i < trackrefs.size(); i++) {
		trackrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
		reflist.push_back(trackrefs[i]);
	}

	XMLClose(str, indent, eol, ind_level);
}

/*--------------------------------------------------------------------------------*/
/** Serialize (or prepare to serialize) additional parts of this object
 *
 * @param dst destination buffer or NULL to calculate required buffer size
 * @param len offset into data to store data, modified by function
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioObject::SerializeEx(uint8_t *dst, uint_t& len) const
{
	uint_t i;

	SerializeTime(dst, len, "startTime", startTime);
	SerializeTime(dst, len, "duration", duration);

	for (i = 0; i < objectrefs.size(); i++) {
		SerializeData(dst, len, objectrefs[i]);
	}

	for (i = 0; i < packformatrefs.size(); i++) {
		SerializeData(dst, len, packformatrefs[i]);
	}

	for (i = 0; i < trackrefs.size(); i++) {
		SerializeData(dst, len, trackrefs[i]);
	}
}

/*----------------------------------------------------------------------------------------------------*/

const string ADMAudioTrack::Type      = "audioTrackUID";
const string ADMAudioTrack::Reference = Type + "Ref";

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
void ADMAudioTrack::DumpEx(map<const ADMObject *,bool>& handledmap, string& str, const string& indent, const string& eol, uint_t ind_level) const
{
	uint_t i;

	Dump(str, indent, eol, ind_level, "trackNum",   trackNum);
	Dump(str, indent, eol, ind_level, "sampleRate", sampleRate);
	Dump(str, indent, eol, ind_level, "bitDepth",   bitDepth);

	for (i = 0; i < trackformatrefs.size(); i++) {
		trackformatrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
	}

	for (i = 0; i < packformatrefs.size(); i++) {
		packformatrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
	}
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML attributes for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrack::XMLAttributes(string& str) const
{
	ADMObject::XMLAttributes(str);

	XMLAttribute(str, "sampleRate", sampleRate);
	XMLAttribute(str, "bitDepth", bitDepth);
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML data for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrack::XMLData(string& str, const string& indent, const string& eol, uint_t ind_level, vector<const ADMObject *>& reflist) const
{
	uint_t i;

	XMLOpen(str, indent, eol, ind_level);
		
	ADMObject::XMLData(str, indent, eol, ind_level, reflist);

	for (i = 0; i < trackformatrefs.size(); i++) {
		//trackformatrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
		reflist.push_back(trackformatrefs[i]);
	}

	for (i = 0; i < packformatrefs.size(); i++) {
		//packformatrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
		reflist.push_back(packformatrefs[i]);
	}

	XMLClose(str, indent, eol, ind_level);
}

bool ADMAudioTrack::Add(ADMAudioTrackFormat *obj)
{
	if (std::find(trackformatrefs.begin(), trackformatrefs.end(), obj) == trackformatrefs.end()) {
		trackformatrefs.push_back(obj);
		return true;
	}

	// reference is already in the list
	return true;
}

bool ADMAudioTrack::Add(ADMAudioPackFormat *obj)
{
	if (std::find(packformatrefs.begin(), packformatrefs.end(), obj) == packformatrefs.end()) {
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

	for (i = 0; i < trackformatrefs.size(); i++) {
		ADMAudioTrackFormat *obj = trackformatrefs[i];

		obj->UpdateLimits();
		Update(obj);
	}

	for (i = 0; i < packformatrefs.size(); i++) {
		ADMAudioPackFormat *obj = packformatrefs[i];

		obj->UpdateLimits();
		Update(obj);
	}
}

/*--------------------------------------------------------------------------------*/
/** Serialize (or prepare to serialize) additional parts of this object
 *
 * @param dst destination buffer or NULL to calculate required buffer size
 * @param len offset into data to store data, modified by function
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrack::SerializeEx(uint8_t *dst, uint_t& len) const
{
	uint_t i;

	SerializeItem(dst, len, "sampleRate", sampleRate);
	SerializeItem(dst, len, "bitDepth", bitDepth);

	for (i = 0; i < trackformatrefs.size(); i++) {
		SerializeData(dst, len, trackformatrefs[i]);
	}

	for (i = 0; i < packformatrefs.size(); i++) {
		SerializeData(dst, len, packformatrefs[i]);
	}
}

/*----------------------------------------------------------------------------------------------------*/

const string ADMAudioPackFormat::Type      = "audioPackFormat";
const string ADMAudioPackFormat::Reference = Type + "IDRef";

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioPackFormat::SetValues()
{
	ADMObject::SetValues();

	SetValue(typeDefinition, "typeDefinition");
}

/*--------------------------------------------------------------------------------*/
/** Add reference to an AudioChannelFormat object
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioPackFormat::Add(ADMAudioChannelFormat *obj)
{
	if (std::find(channelformatrefs.begin(), channelformatrefs.end(), obj) == channelformatrefs.end()) {
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
void ADMAudioPackFormat::DumpEx(map<const ADMObject *,bool>& handledmap, string& str, const string& indent, const string& eol, uint_t ind_level) const
{
	uint_t i;

	Dump(str, indent, eol, ind_level, "typeDefinition", typeDefinition);

	DumpTime(str, indent, eol, ind_level, "start", GetChildrenStartTime());
	DumpTime(str, indent, eol, ind_level, "end",   GetChildrenEndTime());

	for (i = 0; i < channelformatrefs.size(); i++) {
		channelformatrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
	}

	for (i = 0; i < packformatrefs.size(); i++) {
		packformatrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
	}
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML attributes for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioPackFormat::XMLAttributes(string& str) const
{
	ADMObject::XMLAttributes(str);

	XMLAttribute(str, "typeDefinition", typeDefinition);
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML data for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioPackFormat::XMLData(string& str, const string& indent, const string& eol, uint_t ind_level, vector<const ADMObject *>& reflist) const
{
	uint_t i;

	XMLOpen(str, indent, eol, ind_level);

	ADMObject::XMLData(str, indent, eol, ind_level, reflist);

	for (i = 0; i < channelformatrefs.size(); i++) {
		channelformatrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
		reflist.push_back(channelformatrefs[i]);
	}

	for (i = 0; i < packformatrefs.size(); i++) {
		packformatrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
		reflist.push_back(packformatrefs[i]);
	}
	
	XMLClose(str, indent, eol, ind_level);
}

bool ADMAudioPackFormat::Add(ADMAudioPackFormat *obj)
{
	if (std::find(packformatrefs.begin(), packformatrefs.end(), obj) == packformatrefs.end()) {
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

	for (i = 0; i < channelformatrefs.size(); i++) {
		ADMAudioChannelFormat *obj = channelformatrefs[i];

		obj->UpdateLimits();
		Update(obj);
	}
}

/*--------------------------------------------------------------------------------*/
/** Serialize (or prepare to serialize) additional parts of this object
 *
 * @param dst destination buffer or NULL to calculate required buffer size
 * @param len offset into data to store data, modified by function
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioPackFormat::SerializeEx(uint8_t *dst, uint_t& len) const
{
	uint_t i;

	SerializeItem(dst, len, "typeDefinition", typeDefinition);

	for (i = 0; i < channelformatrefs.size(); i++) {
		SerializeData(dst, len, channelformatrefs[i]);
	}

	for (i = 0; i < packformatrefs.size(); i++) {
		SerializeData(dst, len, packformatrefs[i]);
	}
}

/*----------------------------------------------------------------------------------------------------*/

const string ADMAudioStreamFormat::Type      = "audioStreamFormat";
const string ADMAudioStreamFormat::Reference = Type + "IDRef";

/*--------------------------------------------------------------------------------*/
/** Set internal variables from values added to internal list (e.g. from XML)
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::SetValues()
{
	ADMObject::SetValues();
}

/*--------------------------------------------------------------------------------*/
/** Add reference to an AudioChannelFormat object
 */
/*--------------------------------------------------------------------------------*/
bool ADMAudioStreamFormat::Add(ADMAudioChannelFormat *obj)
{
	if (channelformatrefs.size() == 0) {
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
	if (std::find(trackformatrefs.begin(), trackformatrefs.end(), obj) == trackformatrefs.end()) {
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
	if (packformatrefs.size() == 0) {
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

	for (i = 0; i < channelformatrefs.size(); i++) {
		ADMAudioChannelFormat *obj = channelformatrefs[i];

		obj->UpdateLimits();
		Update(obj);
	}

	for (i = 0; i < packformatrefs.size(); i++) {
		ADMAudioPackFormat *obj = packformatrefs[i];

		obj->UpdateLimits();
		Update(obj);
	}
}

/*--------------------------------------------------------------------------------*/
/** Dump additional information about this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::DumpEx(map<const ADMObject *,bool>& handledmap, string& str, const string& indent, const string& eol, uint_t ind_level) const
{
	uint_t i;

	DumpTime(str, indent, eol, ind_level, "start", GetChildrenStartTime());
	DumpTime(str, indent, eol, ind_level, "end",   GetChildrenEndTime());

	for (i = 0; i < channelformatrefs.size(); i++) {
		channelformatrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
	}

	for (i = 0; i < packformatrefs.size(); i++) {
		packformatrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
	}

	for (i = 0; i < trackformatrefs.size(); i++) {
		trackformatrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
	}
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML attributes for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::XMLAttributes(string& str) const
{
	ADMObject::XMLAttributes(str);
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML data for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::XMLData(string& str, const string& indent, const string& eol, uint_t ind_level, vector<const ADMObject *>& reflist) const
{
	uint_t i;

	XMLOpen(str, indent, eol, ind_level);
		
	ADMObject::XMLData(str, indent, eol, ind_level, reflist);

	for (i = 0; i < channelformatrefs.size(); i++) {
		channelformatrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
		reflist.push_back(channelformatrefs[i]);
	}

	for (i = 0; i < packformatrefs.size(); i++) {
		packformatrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
		reflist.push_back(packformatrefs[i]);
	}

	XMLClose(str, indent, eol, ind_level);
}

/*--------------------------------------------------------------------------------*/
/** Serialize (or prepare to serialize) additional parts of this object
 *
 * @param dst destination buffer or NULL to calculate required buffer size
 * @param len offset into data to store data, modified by function
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioStreamFormat::SerializeEx(uint8_t *dst, uint_t& len) const
{
	uint_t i;

	for (i = 0; i < channelformatrefs.size(); i++) {
		SerializeData(dst, len, channelformatrefs[i]);
	}

	for (i = 0; i < packformatrefs.size(); i++) {
		SerializeData(dst, len, packformatrefs[i]);
	}
}

/*----------------------------------------------------------------------------------------------------*/

const string ADMAudioTrackFormat::Type      = "audioTrackFormat";
const string ADMAudioTrackFormat::Reference = Type + "IDRef";

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
void ADMAudioTrackFormat::DumpEx(map<const ADMObject *,bool>& handledmap, string& str, const string& indent, const string& eol, uint_t ind_level) const
{
	uint_t i;

	Dump(str, indent, eol, ind_level, "formatLabel", formatLabel);
	Dump(str, indent, eol, ind_level, "formatDefinition", formatDefinition);

	for (i = 0; i < streamformatrefs.size(); i++) {
		streamformatrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
	}
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML attributes for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrackFormat::XMLAttributes(string& str) const
{
	ADMObject::XMLAttributes(str);

	XMLAttribute(str, "formatLabel",      formatLabel);
	XMLAttribute(str, "formatDefinition", formatDefinition);
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML data for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrackFormat::XMLData(string& str, const string& indent, const string& eol, uint_t ind_level, vector<const ADMObject *>& reflist) const
{
	uint_t i;

	XMLOpen(str, indent, eol, ind_level);
		
	ADMObject::XMLData(str, indent, eol, ind_level, reflist);

	for (i = 0; i < streamformatrefs.size(); i++) {
		streamformatrefs[i]->XMLRef(str, indent, eol, ind_level + 1);
		reflist.push_back(streamformatrefs[i]);
	}

	XMLClose(str, indent, eol, ind_level);
}

bool ADMAudioTrackFormat::Add(ADMAudioStreamFormat *obj)
{
	if (streamformatrefs.size() == 0) {
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

	for (i = 0; i < streamformatrefs.size(); i++) {
		ADMAudioStreamFormat *obj = streamformatrefs[i];

		obj->UpdateLimits();
		Update(obj);
	}
}

/*--------------------------------------------------------------------------------*/
/** Serialize (or prepare to serialize) additional parts of this object
 *
 * @param dst destination buffer or NULL to calculate required buffer size
 * @param len offset into data to store data, modified by function
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioTrackFormat::SerializeEx(uint8_t *dst, uint_t& len) const
{
	uint_t i;

	SerializeItem(dst, len, "formatLabel", formatLabel);

	for (i = 0; i < streamformatrefs.size(); i++) {
		SerializeData(dst, len, streamformatrefs[i]);
	}
}

/*----------------------------------------------------------------------------------------------------*/

const string ADMAudioChannelFormat::Type      = "audioChannelFormat";
const string ADMAudioChannelFormat::Reference = Type + "IDRef";

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
	if (std::find(blockformatrefs.begin(), blockformatrefs.end(), obj) == blockformatrefs.end()) {
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
void ADMAudioChannelFormat::DumpEx(map<const ADMObject *,bool>& handledmap, string& str, const string& indent, const string& eol, uint_t ind_level) const
{
	uint_t i;

	DumpTime(str, indent, eol, ind_level, "start", GetChildrenStartTime());
	DumpTime(str, indent, eol, ind_level, "end",   GetChildrenEndTime());

	for (i = 0; i < blockformatrefs.size(); i++) {
		blockformatrefs[i]->Dump(handledmap, str, indent, eol, ind_level + 1);
	}
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML attributes for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioChannelFormat::XMLAttributes(string& str) const
{
	ADMObject::XMLAttributes(str);
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML data for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioChannelFormat::XMLData(string& str, const string& indent, const string& eol, uint_t ind_level, vector<const ADMObject *>& reflist) const
{
	uint_t i;

	XMLOpen(str, indent, eol, ind_level);

	ADMObject::XMLData(str, indent, eol, ind_level, reflist);

	for (i = 0; i < blockformatrefs.size(); i++) {
		if (i) Printf(str, "%s", eol.c_str());
		blockformatrefs[i]->XMLData(str, indent, eol, ind_level + 1, reflist);
	}
		
	XMLClose(str, indent, eol, ind_level);
}

/*--------------------------------------------------------------------------------*/
/** Serialize (or prepare to serialize) additional parts of this object
 *
 * @param dst destination buffer or NULL to calculate required buffer size
 * @param len offset into data to store data, modified by function
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioChannelFormat::SerializeEx(uint8_t *dst, uint_t& len) const
{
	uint_t i;

	for (i = 0; i < blockformatrefs.size(); i++) {
		SerializeData(dst, len, blockformatrefs[i]);
	}
}

/*----------------------------------------------------------------------------------------------------*/

const string ADMAudioBlockFormat::Type      = "audioBlockFormat";
const string ADMAudioBlockFormat::Reference = Type + "IDRef";

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

	for (it = values.begin(); it != values.end();) {
		const ADMVALUE& value = *it;

		if (value.name == "position") {
			double val;

			if (sscanf(value.value.c_str(), "%lf", &val) > 0) {
				const string *attr;

				if ((attr = GetAttribute(value, "coordinate")) != NULL) {
					DEBUG4(("Position type %s value %0.6lf", attr->c_str(), val));
					if		(*attr == "azimuth")   {position.pos.az = val; position.polar = true;}
					else if (*attr == "elevation") {position.pos.el = val; position.polar = true;}
					else if (*attr == "distance")  {position.pos.d  = val; position.polar = true;}
					else if (*attr == "x") 		   {position.pos.x  = val; position.polar = false;}
					else if (*attr == "y") 		   {position.pos.y  = val; position.polar = false;}
					else if (*attr == "z") 		   {position.pos.z  = val; position.polar = false;}
				}
			}
			else ERROR("Failed to evaluate '%s' as floating point number for position", value.value.c_str());

			it = values.erase(it);
		}
		else if (value.name == "diffuse") {
			supplement.Set(value.name, (value.value == "true"));

			it = values.erase(it);
		}
		else ++it;
	}
}

/*--------------------------------------------------------------------------------*/
/** Dump additional information about this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioBlockFormat::DumpEx(map<const ADMObject *,bool>& handledmap, string& str, const string& indent, const string& eol, uint_t ind_level) const
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
void ADMAudioBlockFormat::XMLAttributes(string& str) const
{
	ADMObject::XMLAttributes(str);

	XMLAttributeTime(str, "rtime", rtime);
	XMLAttributeTime(str, "duration", duration);
}

/*--------------------------------------------------------------------------------*/
/** Output additional XML data for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioBlockFormat::XMLData(string& str, const string& indent, const string& eol, uint_t ind_level, vector<const ADMObject *>& reflist) const
{
	XMLOpen(str, indent, eol, ind_level);

	ADMObject::XMLData(str, indent, eol, ind_level, reflist);

	if (position.polar) {
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
	else {
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
	if (supplement.Get("diffuse", diffuse)) {
		Printf(str, "%s<diffuse>%s</diffuse>%s",
			   CreateIndent(indent, ind_level + 1).c_str(),
			   diffuse ? "true" : "false",
			   eol.c_str());
	}

	XMLClose(str, indent, eol, ind_level);
}

/*--------------------------------------------------------------------------------*/
/** Serialize (or prepare to serialize) additional parts of this object
 *
 * @param dst destination buffer or NULL to calculate required buffer size
 * @param len offset into data to store data, modified by function
 *
 */
/*--------------------------------------------------------------------------------*/
void ADMAudioBlockFormat::SerializeEx(uint8_t *dst, uint_t& len) const
{
	SerializeTime(dst, len, "rtime", rtime);
	SerializeTime(dst, len, "duration", duration);
	SerializeItem(dst, len, "position", position, supplement);
}

/*----------------------------------------------------------------------------------------------------*/

ADMTrackCursor::ADMTrackCursor(const ADMAudioTrack *itrack) : PositionCursor(),
															  track(NULL),
															  blockformats(NULL),
															  blockindex(0)
{
	if (itrack) Setup(itrack);
}

ADMTrackCursor::ADMTrackCursor(const ADMTrackCursor& obj) : PositionCursor(),
															track(obj.track),
															blockformats(obj.blockformats),
															blockindex(obj.blockindex)
{
}

ADMTrackCursor::~ADMTrackCursor()
{
}

void ADMTrackCursor::Setup(const ADMAudioTrack *itrack)
{
	blockformats = NULL;
	blockindex   = 0;

	if ((track = itrack) != NULL) {
		const ADMAudioTrackFormat *trackformat;

		if ((trackformat = track->GetTrackFormatRefs().front()) != NULL) {
			const ADMAudioStreamFormat *streamformat;

			if ((streamformat = trackformat->GetStreamFormatRefs().front()) != NULL) {
				const ADMAudioChannelFormat *channelformat;

				if ((channelformat = streamformat->GetChannelFormatRefs().front()) != NULL) {
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
	track		 = obj.track;
	blockformats = obj.blockformats;
	blockindex   = obj.blockindex;
	return *this;
}

/*--------------------------------------------------------------------------------*/
/** Return position at current time
 */
/*--------------------------------------------------------------------------------*/
const Position *ADMTrackCursor::GetPosition() const
{
	const Position *res = NULL;

	if (blockindex < blockformats->size()) res = &(*blockformats)[blockindex]->GetPosition();

	return res;
}

/*--------------------------------------------------------------------------------*/
/** Return supplementary information at current time
 */
/*--------------------------------------------------------------------------------*/
const ParameterSet *ADMTrackCursor::GetPositionSupplement() const
{
	const ParameterSet *res = NULL;

	if (blockindex < blockformats->size()) res = &(*blockformats)[blockindex]->GetPositionSupplement();

	return res;
}

/*--------------------------------------------------------------------------------*/
/** Get position at specified time (ns)
 */
/*--------------------------------------------------------------------------------*/
bool ADMTrackCursor::Seek(uint64_t t)
{
	uint_t oldindex = blockindex;

	if (blockformats) {
		size_t n = blockformats->size();

		// move blockindex to point to the correct index
		while ((blockindex       > 0) && (t <  (*blockformats)[blockindex]->GetRTime()))     blockindex--;
		while (((blockindex + 1) < n) && (t >= (*blockformats)[blockindex + 1]->GetRTime())) blockindex++;
	}

	return (blockindex != oldindex);
}

BBC_AUDIOTOOLBOX_END
