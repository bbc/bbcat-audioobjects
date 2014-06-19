#ifndef __ADM_OBJECTS__
#define __ADM_OBJECTS__

#include <stdarg.h>

#include <string>
#include <vector>
#include <map>

#include <aplibs-dsp/misc.h>
#include <aplibs-dsp/PositionCursor.h>

BBC_AUDIOTOOLBOX_START

/*----------------------------------------------------------------------------------------------------*/
/** ADM objects
 */
/*----------------------------------------------------------------------------------------------------*/

// ADM parant class (see ADMData.h)
class ADMData;

// forward declarations of some ADM objects
class ADMAudioObject;
class ADMAudioBlockFormat;
class ADMAudioChannelFormat;
class ADMAudioContent;
class ADMAudioPackFormat;
class ADMAudioStreamFormat;
class ADMAudioTrack;
class ADMAudioTrackFormat;

/*--------------------------------------------------------------------------------*/
/** ADM base object
 */
/*--------------------------------------------------------------------------------*/
class ADMObject {
public:
	/*--------------------------------------------------------------------------------*/
	/** Base constructor for all objects
	 *
	 * @param _owner an instance of ADMData that this object should belong to
	 * @param _id unique ID for this object (specified as part of the ADM)
	 * @param _name optional human-friendly name of the object
	 *
	 */
	/*--------------------------------------------------------------------------------*/
	ADMObject(ADMData& _owner, const std::string& _id, const std::string& _name);
	virtual ~ADMObject() {}

	/*--------------------------------------------------------------------------------*/
	/** Returns textual type name of object (must be implemented by derived classes)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const std::string& GetType() const = 0;

	/*--------------------------------------------------------------------------------*/
	/** Returns textual reference type name of object (must be implemented by derived classes)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const std::string& GetReference() const = 0;

	/*--------------------------------------------------------------------------------*/
	/** Set and Get object ID
	 */
	/*--------------------------------------------------------------------------------*/
	void SetID(const std::string& _id) {id= _id;}
	const std::string& GetID()   const {return id;}

	/*--------------------------------------------------------------------------------*/
	/** Set and Get object name (human-friendly)
	 */
	/*--------------------------------------------------------------------------------*/
	void SetName(const std::string& _name) {name= _name;}
	const std::string& GetName() const {return name;}

	/*--------------------------------------------------------------------------------*/
	/** Set and Get object typeLabel
	 */
	/*--------------------------------------------------------------------------------*/
	void SetTypeLabel(const std::string& str) {typeLabel = str;}
	const std::string& GetTypeLabel() const {return typeLabel;}

	/*--------------------------------------------------------------------------------*/
	/** Set internal variables from values added to internal list (e.g. from XML)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SetValues();

	/*--------------------------------------------------------------------------------*/
	/** Try to connect references after all objects have been set up
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SetReferences();

	/*--------------------------------------------------------------------------------*/
	/** Arbitary values list handling
	 */
	/*--------------------------------------------------------------------------------*/
	// each 'value' can also have a list of attributes (a consequence of XML)
	typedef std::map<std::string,std::string> ADMATTRS;
	typedef struct {
		bool		attr;		// true if this value is a simple XML attribute
		std::string name;		// value name
		std::string value;		// value value
		ADMATTRS    attrs;		// additional attributes (if attr == false)
	} ADMVALUE;

	/*--------------------------------------------------------------------------------*/
	/** Add a value to the internal list
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void AddValue(const ADMVALUE& value);

	/*--------------------------------------------------------------------------------*/
	/** Return human friendly summary of the object in the form:
	 *
	 *  <type>/<id> ('<name>')
	 *
	 * This is only used for debugging!
	 */
	/*--------------------------------------------------------------------------------*/
	std::string ToString() const {std::string str; Printf(str, "%s/%s ('%s')", GetType().c_str(), GetID().c_str(), GetName().c_str()); return str;}

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
	virtual void Dump(std::string& str, const std::string& indent = "  ", const std::string& eol = "\n", uint_t ind_level = 0) const;

	/*--------------------------------------------------------------------------------*/
	/** A handler function for the above - DO NOT CALL THIS DIRECTLY, call the above
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void Dump(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const;
	
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
	virtual void GenerateXML(std::string& str, const std::string& indent = "  ", const std::string& eol = "\n", uint_t ind_level = 0) const;

	/*--------------------------------------------------------------------------------*/
	/** Output XML reference information for this object
	 *
	 * @note there is no need to call this function directly, it is called as part of GenerateXML()
	 *
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLRef(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const;

	/*--------------------------------------------------------------------------------*/
	/** Output XML information for this object
	 *
	 * @note there is no need to call this function directly, it is called as part of GenerateXML()
	 *
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLData(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, std::vector<const ADMObject *>& reflist) const;

	/*--------------------------------------------------------------------------------*/
	/** Serialize (or prepare to serialize) this object
	 *
	 * @param dst destination buffer or NULL to calculate required buffer size
	 * @param len offset into data to store data, modified by function
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void Serialize(uint8_t *dst, uint_t& len) const;

	/*--------------------------------------------------------------------------------*/
	/** Convert text time to ns time
	 *
	 * @param t ns time variable to be modified
	 * @param str time in text format 'hh:mm:ss.SSSSS'
	 *
	 * @return true if conversion was successful
	 */
	/*--------------------------------------------------------------------------------*/
	static bool CalcTime(uint64_t& t, const std::string& str);

	/*--------------------------------------------------------------------------------*/
	/** Convert ns time to text time
	 *
	 * @param t ns time
	 *
	 * @return str time in text format 'hh:mm:ss.SSSSS'
	 */
	/*--------------------------------------------------------------------------------*/
	static std::string GenTime(uint64_t t);

	/*--------------------------------------------------------------------------------*/
	/** Convert ns time to samples
	 */
	/*--------------------------------------------------------------------------------*/
	static ulong_t TimeToSamples(uint64_t t, ulong_t sr) {
		return (ulong_t)(((ullong_t)t * (ullong_t)sr) / 1000000000ull);
	}

	/*--------------------------------------------------------------------------------*/
	/** Convert samples to ns time
	 */
	/*--------------------------------------------------------------------------------*/
	static uint64_t SamplesToTime(ulong_t s, ulong_t sr) {
		return (uint64_t)(((ullong_t)s * 1000000000ull) / sr);
	}

	typedef enum {
		SerialDataType_32bit = 0x0,
		SerialDataType_64bit,
		SerialDataType_double,

		SerialDataType_Time_ns,
		SerialDataType_String,
		SerialDataType_Position,
		SerialDataType_Position_Supplement,
		SerialDataType_Reference,

		SerialDataType_Values_And_Attributes = 0x70,
		SerialDataType_Attribute,
		SerialDataType_Value,
		SerialDataType_Value_Attributes,
		SerialDataType_Value_Attribute,

		SerialDataType_ADMHeader = 0x80,
		SerialDataType_ObjectCRC,
		SerialDataType_Programme,
		SerialDataType_Content,
		SerialDataType_Object,
		SerialDataType_TrackUID,
		SerialDataType_PackFormat,
		SerialDataType_StreamFormat,
		SerialDataType_ChannelFormat,
		SerialDataType_BlockFormat,
		SerialDataType_TrackFormat,
	} SerialDataType_t;

	virtual SerialDataType_t GetSerialDataType() const = 0;

protected:
	/*--------------------------------------------------------------------------------*/
	/** Register this object with the owner
	 */
	/*--------------------------------------------------------------------------------*/
	void Register();

	typedef std::vector<ADMVALUE> ADMVALUES;

	/*--------------------------------------------------------------------------------*/
	/** Return ptr to value with specified name or NULL
	 */
	/*--------------------------------------------------------------------------------*/
	const ADMVALUE *GetValue(const std::string& name) const;

	/*--------------------------------------------------------------------------------*/
	/** Return ptr to attributes within specified value with specified name or NULL
	 */
	/*--------------------------------------------------------------------------------*/
	const std::string *GetAttribute(const ADMVALUE& value, const std::string& name) const;

	/*--------------------------------------------------------------------------------*/
	/** Remove and delete value from internal list of values
	 */
	/*--------------------------------------------------------------------------------*/
	void EraseValue(const ADMVALUE *value);

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
	bool SetValue(std::string& res, const std::string& name);
	bool SetValue(double& res, const std::string& name);
	bool SetValue(uint_t& res, const std::string& name);
	bool SetValue(ulong_t& res, const std::string& name);
	bool SetValue(sint_t& res, const std::string& name);
	bool SetValue(slong_t& res, const std::string& name);
	bool SetValue(bool& res, const std::string& name);
	bool SetValueTime(uint64_t& res, const std::string& name);

	/*--------------------------------------------------------------------------------*/
	/** Prototypes for adding references of different kinds.  Any unimplemented handlers will result in references of that kind to be rejected
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool Add(ADMAudioContent       *obj) {UNUSED_PARAMETER(obj); return false;}
	virtual bool Add(ADMAudioObject		   *obj) {UNUSED_PARAMETER(obj); return false;}
	virtual bool Add(ADMAudioTrack         *obj) {UNUSED_PARAMETER(obj); return false;}
	virtual bool Add(ADMAudioPackFormat    *obj) {UNUSED_PARAMETER(obj); return false;}
	virtual bool Add(ADMAudioStreamFormat  *obj) {UNUSED_PARAMETER(obj); return false;}
	virtual bool Add(ADMAudioChannelFormat *obj) {UNUSED_PARAMETER(obj); return false;}
	virtual bool Add(ADMAudioTrackFormat   *obj) {UNUSED_PARAMETER(obj); return false;}
	virtual bool Add(ADMAudioBlockFormat   *obj) {UNUSED_PARAMETER(obj); return false;}

	/*--------------------------------------------------------------------------------*/
	/** Individual variable type dumping helper functions
	 */
	/*--------------------------------------------------------------------------------*/
	static void Dump(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, const std::string& name, const std::string& value);
	static void Dump(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, const std::string& name, uint_t value);
	static void Dump(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, const std::string& name, double value);
	static void Dump(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, const std::string& name, bool value);
	static void DumpTime(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, const std::string& name, uint64_t value);

	/*--------------------------------------------------------------------------------*/
	/** Internal dumping functions implemented by each derived object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void DumpEx(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const {
		UNUSED_PARAMETER(handledmap);
		UNUSED_PARAMETER(str);
		UNUSED_PARAMETER(indent);
		UNUSED_PARAMETER(eol);
		UNUSED_PARAMETER(ind_level);
	}

	/*--------------------------------------------------------------------------------*/
	/** Individual variable type XML attribute helper functions
	 */
	/*--------------------------------------------------------------------------------*/
	static void XMLAttribute(std::string& str, const std::string& name, const std::string& value);
	static void XMLAttribute(std::string& str, const std::string& name, uint_t value);
	static void XMLAttribute(std::string& str, const std::string& name, double value);
	static void XMLAttributeTime(std::string& str, const std::string& name, uint64_t value);

	/*--------------------------------------------------------------------------------*/
	/** 'Open' (start) an XML section
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLOpen(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const;

	/*--------------------------------------------------------------------------------*/
	/** Output XML attributes (within the opening section), expanded by derived classes
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLAttributes(std::string& str) const;

	/*--------------------------------------------------------------------------------*/
	/** Return true if there is no additional XML data to output (i.e. the XML close can be on the same line as the open)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool XMLEmpty() const {return true;}

	/*--------------------------------------------------------------------------------*/
	/** Close an XML section
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLClose(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const;

	/*--------------------------------------------------------------------------------*/
	/** Serialize (or prepare to serialize) additional parts of this object
	 *
	 * @param dst destination buffer or NULL to calculate required buffer size
	 * @param len offset into data to store data, modified by function
	 *
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SerializeEx(uint8_t *dst, uint_t& len) const {
		UNUSED_PARAMETER(dst);
		UNUSED_PARAMETER(len);
	}

	friend class ADMData;

	/*--------------------------------------------------------------------------------*/
	/** Serialize a sync block to aid syncing (random 32-bit number followed by 32-bit CRC)
	 *
	 * @param dst destination buffer or NULL to calculate required buffer size
	 * @param len offset into data to store data, modified by function
	 *
	 */
	/*--------------------------------------------------------------------------------*/
	static void SerializeSync(uint8_t *dst, uint_t& len, uint_t len0);

	/*--------------------------------------------------------------------------------*/
	/** Add data to a serilization buffer (or calculate size of data)
	 *
	 * @param dst buffer to receive data (or NULL to just calculate size)
	 * @param len offset into buffer, modified by this function
	 * @param obj object to store
	 *
	 */
	/*--------------------------------------------------------------------------------*/
	static void SerializeData(uint8_t *dst, uint_t& len, const void *obj, uint_t objlen, bool byteswap = false);
	static void SerializeData(uint8_t *dst, uint_t& len, bool obj) {SerializeData(dst, len, (uint8_t)obj);}
	static void SerializeData(uint8_t *dst, uint_t& len, uint8_t obj);
	static void SerializeData(uint8_t *dst, uint_t& len, uint16_t obj);
	static void SerializeData(uint8_t *dst, uint_t& len, uint32_t obj, uint_t bytes = sizeof(uint32_t));
	static void SerializeData(uint8_t *dst, uint_t& len, uint64_t obj, uint_t bytes = sizeof(uint64_t));
	static void SerializeData(uint8_t *dst, uint_t& len, double obj);
	static void SerializeData(uint8_t *dst, uint_t& len, SerialDataType_t type);
	static void SerializeData(uint8_t *dst, uint_t& len, SerialDataType_t type, uint_t sublen);
	static void SerializeData(uint8_t *dst, uint_t& len, const std::string& obj);
	static void SerializeData(uint8_t *dst, uint_t& len, const ADMATTRS& obj);
	static void SerializeData(uint8_t *dst, uint_t& len, const ADMVALUE& obj);
	static void SerializeData(uint8_t *dst, uint_t& len, const ADMVALUES& obj);
	static void SerializeData(uint8_t *dst, uint_t& len, const ADMObject *obj);
	static void SerializeData(uint8_t *dst, uint_t& len, const Position& obj);
	static void SerializeData(uint8_t *dst, uint_t& len, const ParameterSet& obj);
	static void SerializeObjectCRC(uint8_t *dst, uint_t& len, uint_t len0);

	static void SerializeItem(uint8_t *dst, uint_t& len, const std::string& name, uint32_t obj);
	static void SerializeItem(uint8_t *dst, uint_t& len, const std::string& name, uint64_t obj);
	static void SerializeItem(uint8_t *dst, uint_t& len, const std::string& name, double obj);
	static void SerializeTime(uint8_t *dst, uint_t& len, const std::string& name, uint64_t obj);
	static void SerializeItem(uint8_t *dst, uint_t& len, const std::string& name, const std::string& obj);
	static void SerializeItem(uint8_t *dst, uint_t& len, const std::string& name, const Position& obj, const ParameterSet& obj2);

protected:
	ADMData&    owner;
	std::string id;
	std::string name;
	std::string typeLabel;
	ADMVALUES   values;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMLevelObject {
public:
	ADMLevelObject() : level(1.0) {}

	/*--------------------------------------------------------------------------------*/
	/** Set and Get audio level
	 */
	/*--------------------------------------------------------------------------------*/
	void   SetLevel(double l = 1.0) {level = l;}
	double GetLevel() const {return level;}

protected:
	double level;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMTimeObject {
public:
	ADMTimeObject() : childrenStartTime(~(uint64_t)0),
					  childrenEndTime(0) {}

	/*--------------------------------------------------------------------------------*/
	/** Update time limits
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void UpdateLimits() {}

	/*--------------------------------------------------------------------------------*/
	/** Reset and re-calculate limits
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void Reset() {
		childrenStartTime = ~(uint64_t)0;
		childrenEndTime   = 0;
		UpdateLimits();
	}

	/*--------------------------------------------------------------------------------*/
	/** Update time limits
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void Update(uint64_t t1, uint64_t t2) {
		childrenStartTime = MIN(childrenStartTime, t1);
		childrenEndTime   = MAX(childrenEndTime, t2);
	}

	/*--------------------------------------------------------------------------------*/
	/** Update time limits
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void Update(const ADMTimeObject *obj) {Update(obj->GetChildrenStartTime(), obj->GetChildrenEndTime());}

	/*--------------------------------------------------------------------------------*/
	/** Return first start point of all child objects
	 */
	/*--------------------------------------------------------------------------------*/
	uint64_t GetChildrenStartTime() const {return MIN(childrenStartTime, childrenEndTime);}

	/*--------------------------------------------------------------------------------*/
	/** Return last end point of all child objects
	 */
	/*--------------------------------------------------------------------------------*/
	uint64_t GetChildrenEndTime() const {return childrenEndTime;}

protected:
	uint64_t childrenStartTime;
	uint64_t childrenEndTime;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMAudioProgramme : public ADMObject, public ADMLevelObject {
public:
	/*--------------------------------------------------------------------------------*/
	/** ADM AudioProgramme object
	 *
	 * @param _owner an instance of ADMData that this object should belong to
	 * @param _id unique ID for this object (specified as part of the ADM)
	 * @param _name optional human-friendly name of the object
	 *
	 * @note type passed to base constructor is fixed by static member variable Type 
	 */
	/*--------------------------------------------------------------------------------*/
	ADMAudioProgramme(ADMData& _owner, const std::string& _id, const std::string& _name) : ADMObject(_owner, _id, _name),
																						   ADMLevelObject() {Register();}

	/*--------------------------------------------------------------------------------*/
	/** Return textual type name of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const std::string& GetType() const {return Type;}

	/*--------------------------------------------------------------------------------*/
	/** Returns textual reference type name of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const std::string& GetReference() const {return Reference;}

	/*--------------------------------------------------------------------------------*/
	/** Return serial data type of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual SerialDataType_t GetSerialDataType() const {return SerialDataType_Programme;}

	/*--------------------------------------------------------------------------------*/
	/** Set internal variables from values added to internal list (e.g. from XML)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SetValues();

	/*--------------------------------------------------------------------------------*/
	/** Set and Get language
	 */
	/*--------------------------------------------------------------------------------*/
	void SetLanguage(const std::string& str) {language = str;}
	const std::string& GetLanguage() const {return language;}

	/*--------------------------------------------------------------------------------*/
	/** Add reference to an AudioContent object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool Add(ADMAudioContent *obj);
	/*--------------------------------------------------------------------------------*/
	/** Return list of AudioContents
	 */
	/*--------------------------------------------------------------------------------*/
	const std::vector<ADMAudioContent *>& GetContentRefs() const {return contentrefs;}

	/*--------------------------------------------------------------------------------*/
	/** Output additional XML data for this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLData(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, std::vector<const ADMObject *>& reflist) const;

	// static type name
	static const std::string Type;

	// static type reference name
	static const std::string Reference;

protected:
	/*--------------------------------------------------------------------------------*/
	/** Dump additional information about this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void DumpEx(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const;

	/*--------------------------------------------------------------------------------*/
	/** Output additional XML attributes for this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLAttributes(std::string& str) const;

	/*--------------------------------------------------------------------------------*/
	/** Return true if there is no additional XML data to output (i.e. the XML close can be on the same line as the open)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool XMLEmpty() const {return (ADMObject::XMLEmpty() && (contentrefs.size() == 0));}

	/*--------------------------------------------------------------------------------*/
	/** Serialize (or prepare to serialize) additional parts of this object
	 *
	 * @param dst destination buffer or NULL to calculate required buffer size
	 * @param len offset into data to store data, modified by function
	 *
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SerializeEx(uint8_t *dst, uint_t& len) const;

protected:
	std::vector<ADMAudioContent *> contentrefs;
	std::string				       language;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMAudioContent : public ADMObject, public ADMLevelObject {
public:
	/*--------------------------------------------------------------------------------*/
	/** ADM AudioContent object
	 *
	 * @param _owner an instance of ADMData that this object should belong to
	 * @param _id unique ID for this object (specified as part of the ADM)
	 * @param _name optional human-friendly name of the object
	 *
	 * @note type passed to base constructor is fixed by static member variable Type 
	 */
	/*--------------------------------------------------------------------------------*/
	ADMAudioContent(ADMData& _owner, const std::string& _id, const std::string& _name) : ADMObject(_owner, _id, _name),
																						 ADMLevelObject() {Register();}

	/*--------------------------------------------------------------------------------*/
	/** Return textual type name of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const std::string& GetType() const {return Type;}

	/*--------------------------------------------------------------------------------*/
	/** Returns textual reference type name of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const std::string& GetReference() const {return Reference;}

	/*--------------------------------------------------------------------------------*/
	/** Return serial data type of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual SerialDataType_t GetSerialDataType() const {return SerialDataType_Content;}

	/*--------------------------------------------------------------------------------*/
	/** Set internal variables from values added to internal list (e.g. from XML)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SetValues();

	/*--------------------------------------------------------------------------------*/
	/** Set and Get language
	 */
	/*--------------------------------------------------------------------------------*/
	void SetLanguage(const std::string& str) {language = str;}
	const std::string& GetLanguage() const {return language;}

	/*--------------------------------------------------------------------------------*/
	/** Add reference to an AudioObject object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool Add(ADMAudioObject *obj);
	/*--------------------------------------------------------------------------------*/
	/** Return list of AudioObjects
	 */
	/*--------------------------------------------------------------------------------*/
	const std::vector<ADMAudioObject *>& GetObjectRefs() const {return objectrefs;}

	/*--------------------------------------------------------------------------------*/
	/** Output additional XML data for this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLData(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, std::vector<const ADMObject *>& reflist) const;

	// static type name
	static const std::string Type;

	// static type reference name
	static const std::string Reference;

protected:
	/*--------------------------------------------------------------------------------*/
	/** Dump additional information about this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void DumpEx(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const;

	/*--------------------------------------------------------------------------------*/
	/** Output additional XML attributes for this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLAttributes(std::string& str) const;

	/*--------------------------------------------------------------------------------*/
	/** Return true if there is no additional XML data to output (i.e. the XML close can be on the same line as the open)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool XMLEmpty() const {return (ADMObject::XMLEmpty() && (objectrefs.size() == 0));}

	/*--------------------------------------------------------------------------------*/
	/** Serialize (or prepare to serialize) additional parts of this object
	 *
	 * @param dst destination buffer or NULL to calculate required buffer size
	 * @param len offset into data to store data, modified by function
	 *
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SerializeEx(uint8_t *dst, uint_t& len) const;

protected:
	std::string					  language;
	std::vector<ADMAudioObject *> objectrefs;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMAudioObject : public ADMObject, public ADMLevelObject, public ADMTimeObject {
public:
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
	ADMAudioObject(ADMData& _owner, const std::string& _id, const std::string& _name);

	/*--------------------------------------------------------------------------------*/
	/** Return textual type name of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const std::string& GetType() const {return Type;}

	/*--------------------------------------------------------------------------------*/
	/** Returns textual reference type name of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const std::string& GetReference() const {return Reference;}

	/*--------------------------------------------------------------------------------*/
	/** Return serial data type of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual SerialDataType_t GetSerialDataType() const {return SerialDataType_Object;}

	/*--------------------------------------------------------------------------------*/
	/** Set internal variables from values added to internal list (e.g. from XML)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SetValues();

	/*--------------------------------------------------------------------------------*/
	/** Set and Get startTime
	 */
	/*--------------------------------------------------------------------------------*/
	void     SetStartTime(uint64_t t) {startTime = t; childrenEndTime = MAX(childrenEndTime, startTime);}
	uint64_t GetStartTime() const {return startTime;}

	/*--------------------------------------------------------------------------------*/
	/** Set and Get duration
	 */
	/*--------------------------------------------------------------------------------*/
	void     SetDuration(uint64_t t) {duration = t;}
	uint64_t GetDuration() const {return duration;}

	/*--------------------------------------------------------------------------------*/
	/** Add reference to an AudioObject object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool Add(ADMAudioObject *obj);
	/*--------------------------------------------------------------------------------*/
	/** Return list of AudioObjects
	 */
	/*--------------------------------------------------------------------------------*/
	const std::vector<ADMAudioObject *>& GetObjectRefs() const {return objectrefs;}
	/*--------------------------------------------------------------------------------*/
	/** Add reference to an AudioPackFormat object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool Add(ADMAudioPackFormat *obj);
	/*--------------------------------------------------------------------------------*/
	/** Return list of PackFormats
	 */
	/*--------------------------------------------------------------------------------*/
	const std::vector<ADMAudioPackFormat *>& GetPackFormatRefs() const {return packformatrefs;}
	/*--------------------------------------------------------------------------------*/
	/** Add reference to an AudioTrack object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool Add(ADMAudioTrack *obj);
	/*--------------------------------------------------------------------------------*/
	/** Return list of AudioTracks
	 */
	/*--------------------------------------------------------------------------------*/
	const std::vector<ADMAudioTrack *>& GetTrackRefs() const {return trackrefs;}

	/*--------------------------------------------------------------------------------*/
	/** Update track and time limits after all references have been set
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void UpdateLimits();

	uint_t   GetChildrenStartChannel() const {return MIN(childrenMinChannel, childrenMaxChannel);}
	uint_t   GetChildrenChannelCount() const {return childrenMaxChannel + 1 - GetChildrenStartChannel();}

	/*--------------------------------------------------------------------------------*/
	/** Output additional XML data for this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLData(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, std::vector<const ADMObject *>& reflist) const;

	static bool Compare(const ADMAudioObject *obj1, const ADMAudioObject *obj2) {
		return ((obj1->GetChildrenStartTime() < obj2->GetChildrenStartTime()) ||
				((obj1->GetChildrenStartTime() == obj2->GetChildrenStartTime()) &&
				 (obj1->GetChildrenStartChannel() < obj2->GetChildrenStartChannel())));
	}

	// static type name
	static const std::string Type;

	// static type reference name
	static const std::string Reference;

protected:
	/*--------------------------------------------------------------------------------*/
	/** Dump additional information about this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void DumpEx(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const;

	/*--------------------------------------------------------------------------------*/
	/** Output additional XML attributes for this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLAttributes(std::string& str) const;

	/*--------------------------------------------------------------------------------*/
	/** Return true if there is no additional XML data to output (i.e. the XML close can be on the same line as the open)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool XMLEmpty() const {return (ADMObject::XMLEmpty() && (objectrefs.size() == 0) && (packformatrefs.size() == 0) && (trackrefs.size() == 0));}

	/*--------------------------------------------------------------------------------*/
	/** Serialize (or prepare to serialize) additional parts of this object
	 *
	 * @param dst destination buffer or NULL to calculate required buffer size
	 * @param len offset into data to store data, modified by function
	 *
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SerializeEx(uint8_t *dst, uint_t& len) const;

protected:
	std::vector<ADMAudioObject     *> objectrefs;
	std::vector<ADMAudioPackFormat *> packformatrefs;
	std::vector<ADMAudioTrack	   *> trackrefs;
	uint64_t 		  			   startTime;
	uint64_t 		  			   duration;
	uint_t						   childrenMinChannel;
	uint_t						   childrenMaxChannel;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMAudioTrack : public ADMObject, public ADMLevelObject, public ADMTimeObject {
public:
	/*--------------------------------------------------------------------------------*/
	/** ADM AudioTrack object
	 *
	 * @param _owner an instance of ADMData that this object should belong to
	 * @param _id unique ID for this object (specified as part of the ADM)
	 * @param _name optional human-friendly name of the object
	 *
	 * @note type passed to base constructor is fixed by static member variable Type 
	 */
	/*--------------------------------------------------------------------------------*/
	ADMAudioTrack(ADMData& _owner, const std::string& _id, const std::string& _name) : ADMObject(_owner, _id, _name),
																					   ADMLevelObject(),
																					   ADMTimeObject(),
																					   trackNum(0),
																					   sampleRate(0),
																					   bitDepth(0) {Register();}

	/*--------------------------------------------------------------------------------*/
	/** Return textual type name of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const std::string& GetType() const {return Type;}

	/*--------------------------------------------------------------------------------*/
	/** Returns textual reference type name of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const std::string& GetReference() const {return Reference;}

	/*--------------------------------------------------------------------------------*/
	/** Return serial data type of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual SerialDataType_t GetSerialDataType() const {return SerialDataType_TrackUID;}

	/*--------------------------------------------------------------------------------*/
	/** Set internal variables from values added to internal list (e.g. from XML)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SetValues();

	/*--------------------------------------------------------------------------------*/
	/** Set and Get track index
	 */
	/*--------------------------------------------------------------------------------*/
	void SetTrackNum(uint_t ind) {trackNum = ind;}
	uint_t GetTrackNum() const {return trackNum;}

	/*--------------------------------------------------------------------------------*/
	/** Set and Get sample rate
	 */
	/*--------------------------------------------------------------------------------*/
	void SetSampleRate(uint32_t sr) {sampleRate = sr;}
	uint32_t GetSampleRate() const {return sampleRate;}

	/*--------------------------------------------------------------------------------*/
	/** Set and Get bit depth
	 */
	/*--------------------------------------------------------------------------------*/
	void SetBitDepth(uint_t bd) {bitDepth = bd;}
	uint_t GetBitDepth() const {return bitDepth;}

	/*--------------------------------------------------------------------------------*/
	/** Add reference to an AudioTrackFormat object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool Add(ADMAudioTrackFormat *obj);
	/*--------------------------------------------------------------------------------*/
	/** Return list of AudioTrackFormats
	 */
	/*--------------------------------------------------------------------------------*/
	const std::vector<ADMAudioTrackFormat *>& GetTrackFormatRefs() const {return trackformatrefs;}
	/*--------------------------------------------------------------------------------*/
	/** Add reference to an AudioPackFormat object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool Add(ADMAudioPackFormat *obj);
	/*--------------------------------------------------------------------------------*/
	/** Return list of AudioPackFormats
	 */
	/*--------------------------------------------------------------------------------*/
	const std::vector<ADMAudioPackFormat *>& GetPackFormatRefs() const {return packformatrefs;}

	/*--------------------------------------------------------------------------------*/
	/** Update time limits
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void UpdateLimits();

	/*--------------------------------------------------------------------------------*/
	/** Output additional XML data for this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLData(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, std::vector<const ADMObject *>& reflist) const;

	/*--------------------------------------------------------------------------------*/
	/** Convert ns time to samples
	 */
	/*--------------------------------------------------------------------------------*/
	ulong_t TimeToSamples(uint64_t t) const {
		return (ulong_t)(((ullong_t)t * (ullong_t)sampleRate) / 1000000000ull);
	}

	/*--------------------------------------------------------------------------------*/
	/** Convert samples to ns time
	 */
	/*--------------------------------------------------------------------------------*/
	uint64_t SamplesToTime(ulong_t s) const {
		return (uint64_t)(((ullong_t)s * 1000000000ull) / sampleRate);
	}

	static bool Compare(const ADMAudioTrack *track1, const ADMAudioTrack *track2) {
		return (track1->trackNum < track2->trackNum);
	}

	// static type name
	static const std::string Type;

	// static type reference name
	static const std::string Reference;

protected:
	/*--------------------------------------------------------------------------------*/
	/** Dump additional information about this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void DumpEx(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const;

	/*--------------------------------------------------------------------------------*/
	/** Output additional XML attributes for this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLAttributes(std::string& str) const;

	/*--------------------------------------------------------------------------------*/
	/** Return true if there is no additional XML data to output (i.e. the XML close can be on the same line as the open)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool XMLEmpty() const {return (ADMObject::XMLEmpty() && (trackformatrefs.size() == 0) && (packformatrefs.size() == 0));}

	/*--------------------------------------------------------------------------------*/
	/** Serialize (or prepare to serialize) additional parts of this object
	 *
	 * @param dst destination buffer or NULL to calculate required buffer size
	 * @param len offset into data to store data, modified by function
	 *
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SerializeEx(uint8_t *dst, uint_t& len) const;

protected:
	uint_t	       				  	   trackNum;
	uint32_t       				  	   sampleRate;
	uint_t         				  	   bitDepth;
	std::vector<ADMAudioTrackFormat *> trackformatrefs;
	std::vector<ADMAudioPackFormat *>  packformatrefs;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMAudioPackFormat : public ADMObject, public ADMTimeObject {
public:
	/*--------------------------------------------------------------------------------*/
	/** ADM AudioPackFormat object
	 *
	 * @param _owner an instance of ADMData that this object should belong to
	 * @param _id unique ID for this object (specified as part of the ADM)
	 * @param _name optional human-friendly name of the object
	 *
	 * @note type passed to base constructor is fixed by static member variable Type 
	 */
	/*--------------------------------------------------------------------------------*/
	ADMAudioPackFormat(ADMData& _owner, const std::string& _id, const std::string& _name) : ADMObject(_owner, _id, _name),
																							ADMTimeObject() {Register();}

	/*--------------------------------------------------------------------------------*/
	/** Return textual type name of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const std::string& GetType() const {return Type;}

	/*--------------------------------------------------------------------------------*/
	/** Returns textual reference type name of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const std::string& GetReference() const {return Reference;}

	/*--------------------------------------------------------------------------------*/
	/** Return serial data type of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual SerialDataType_t GetSerialDataType() const {return SerialDataType_PackFormat;}

	/*--------------------------------------------------------------------------------*/
	/** Set internal variables from values added to internal list (e.g. from XML)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SetValues();

	void SetTypeDefinition(const std::string& str) {typeDefinition = str;}
	const std::string& GetTypeDefinition() const {return typeDefinition;}

	/*--------------------------------------------------------------------------------*/
	/** Add reference to an AudioChannelFormat object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool Add(ADMAudioChannelFormat *obj);
	/*--------------------------------------------------------------------------------*/
	/** Return list of AudioChannelFormats
	 */
	/*--------------------------------------------------------------------------------*/
	const std::vector<ADMAudioChannelFormat *>& GetChannelFormatRefs() const {return channelformatrefs;}
	/*--------------------------------------------------------------------------------*/
	/** Add reference to an AudioPackFormat object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool Add(ADMAudioPackFormat *obj);
	/*--------------------------------------------------------------------------------*/
	/** Return list of AudioPackFormats
	 */
	/*--------------------------------------------------------------------------------*/
	const std::vector<ADMAudioPackFormat *>& GetPackFormatRefs() const {return packformatrefs;}

	/*--------------------------------------------------------------------------------*/
	/** Update time limits
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void UpdateLimits();

	/*--------------------------------------------------------------------------------*/
	/** Output additional XML data for this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLData(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, std::vector<const ADMObject *>& reflist) const;

	// static type name
	static const std::string Type;

	// static type reference name
	static const std::string Reference;

protected:
	/*--------------------------------------------------------------------------------*/
	/** Dump additional information about this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void DumpEx(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const;

	/*--------------------------------------------------------------------------------*/
	/** Output additional XML attributes for this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLAttributes(std::string& str) const;

	/*--------------------------------------------------------------------------------*/
	/** Return true if there is no additional XML data to output (i.e. the XML close can be on the same line as the open)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool XMLEmpty() const {return (ADMObject::XMLEmpty() && (channelformatrefs.size() == 0) && (packformatrefs.size() == 0));}

	/*--------------------------------------------------------------------------------*/
	/** Serialize (or prepare to serialize) additional parts of this object
	 *
	 * @param dst destination buffer or NULL to calculate required buffer size
	 * @param len offset into data to store data, modified by function
	 *
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SerializeEx(uint8_t *dst, uint_t& len) const;

protected:
	std::vector<ADMAudioChannelFormat *> channelformatrefs;
	std::vector<ADMAudioPackFormat	  *> packformatrefs;
	std::string typeDefinition;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMAudioStreamFormat : public ADMObject, public ADMTimeObject {
public:
	/*--------------------------------------------------------------------------------*/
	/** ADM AudioStreamFormat object
	 *
	 * @param _owner an instance of ADMData that this object should belong to
	 * @param _id unique ID for this object (specified as part of the ADM)
	 * @param _name optional human-friendly name of the object
	 *
	 * @note type passed to base constructor is fixed by static member variable Type 
	 */
	/*--------------------------------------------------------------------------------*/
	ADMAudioStreamFormat(ADMData& _owner, const std::string& _id, const std::string& _name) : ADMObject(_owner, _id, _name),
																							  ADMTimeObject() {Register();}

	/*--------------------------------------------------------------------------------*/
	/** Return textual type name of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const std::string& GetType() const {return Type;}

	/*--------------------------------------------------------------------------------*/
	/** Returns textual reference type name of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const std::string& GetReference() const {return Reference;}

	/*--------------------------------------------------------------------------------*/
	/** Return serial data type of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual SerialDataType_t GetSerialDataType() const {return SerialDataType_StreamFormat;}

	/*--------------------------------------------------------------------------------*/
	/** Set internal variables from values added to internal list (e.g. from XML)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SetValues();

	/*--------------------------------------------------------------------------------*/
	/** Add reference to an AudioChannelFormat object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool Add(ADMAudioChannelFormat *obj);
	/*--------------------------------------------------------------------------------*/
	/** Return list of AudioChannelFormats
	 */
	/*--------------------------------------------------------------------------------*/
	const std::vector<ADMAudioChannelFormat *>& GetChannelFormatRefs() const {return channelformatrefs;}

	/*--------------------------------------------------------------------------------*/
	/** Add reference to an AudioTrackFormat object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool Add(ADMAudioTrackFormat *obj);
	/*--------------------------------------------------------------------------------*/
	/** Return list of AudioTrackFormats
	 */
	/*--------------------------------------------------------------------------------*/
	const std::vector<ADMAudioTrackFormat *>& GetTrackFormatRefs() const {return trackformatrefs;}

	/*--------------------------------------------------------------------------------*/
	/** Add reference to an AudioPackFormat object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool Add(ADMAudioPackFormat *obj);
	/*--------------------------------------------------------------------------------*/
	/** Return list of AudioPackFormats
	 */
	/*--------------------------------------------------------------------------------*/
	const std::vector<ADMAudioPackFormat *>& GetPackFormatRefs() const {return packformatrefs;}

	/*--------------------------------------------------------------------------------*/
	/** Update time limits
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void UpdateLimits();

	/*--------------------------------------------------------------------------------*/
	/** Output additional XML data for this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLData(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, std::vector<const ADMObject *>& reflist) const;

	// static type name
	static const std::string Type;

	// static type reference name
	static const std::string Reference;

protected:
	/*--------------------------------------------------------------------------------*/
	/** Dump additional information about this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void DumpEx(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const;

	/*--------------------------------------------------------------------------------*/
	/** Output additional XML attributes for this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLAttributes(std::string& str) const;

	/*--------------------------------------------------------------------------------*/
	/** Return true if there is no additional XML data to output (i.e. the XML close can be on the same line as the open)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool XMLEmpty() const {return (ADMObject::XMLEmpty() && (channelformatrefs.size() == 0) && (trackformatrefs.size() == 0) && (packformatrefs.size() == 0));}

	/*--------------------------------------------------------------------------------*/
	/** Serialize (or prepare to serialize) additional parts of this object
	 *
	 * @param dst destination buffer or NULL to calculate required buffer size
	 * @param len offset into data to store data, modified by function
	 *
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SerializeEx(uint8_t *dst, uint_t& len) const;

protected:
	std::vector<ADMAudioChannelFormat *> channelformatrefs;
	std::vector<ADMAudioTrackFormat *>   trackformatrefs;
	std::vector<ADMAudioPackFormat *>    packformatrefs;
};

class ADMAudioChannelFormat : public ADMObject, public ADMTimeObject {
public:
	/*--------------------------------------------------------------------------------*/
	/** ADM AudioChannelFormat object
	 *
	 * @param _owner an instance of ADMData that this object should belong to
	 * @param _id unique ID for this object (specified as part of the ADM)
	 * @param _name optional human-friendly name of the object
	 *
	 * @note type passed to base constructor is fixed by static member variable Type 
	 */
	/*--------------------------------------------------------------------------------*/
	ADMAudioChannelFormat(ADMData& _owner, const std::string& _id, const std::string& _name) : ADMObject(_owner, _id, _name),
																							   ADMTimeObject() {Register();}

	/*--------------------------------------------------------------------------------*/
	/** Return textual type name of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const std::string& GetType() const {return Type;}

	/*--------------------------------------------------------------------------------*/
	/** Returns textual reference type name of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const std::string& GetReference() const {return Reference;}

	/*--------------------------------------------------------------------------------*/
	/** Return serial data type of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual SerialDataType_t GetSerialDataType() const {return SerialDataType_ChannelFormat;}

	/*--------------------------------------------------------------------------------*/
	/** Set internal variables from values added to internal list (e.g. from XML)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SetValues();

	/*--------------------------------------------------------------------------------*/
	/** Add reference to an AudioBlockFormat object and ensures blocks are sorted by time
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool Add(ADMAudioBlockFormat *obj);
	/*--------------------------------------------------------------------------------*/
	/** Return list of AudioBlockFormats
	 */
	/*--------------------------------------------------------------------------------*/
	const std::vector<ADMAudioBlockFormat *>& GetBlockFormatRefs() const {return blockformatrefs;}

	/*--------------------------------------------------------------------------------*/
	/** Empty functions (to disable parent functionality)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void UpdateLimits() {}
	virtual void Reset() {}

	/*--------------------------------------------------------------------------------*/
	/** Output additional XML data for this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLData(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, std::vector<const ADMObject *>& reflist) const;

	// static type name
	static const std::string Type;

	// static type reference name
	static const std::string Reference;

protected:
	/*--------------------------------------------------------------------------------*/
	/** Dump additional information about this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void DumpEx(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const;

	/*--------------------------------------------------------------------------------*/
	/** Output additional XML attributes for this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLAttributes(std::string& str) const;

	/*--------------------------------------------------------------------------------*/
	/** Return true if there is no additional XML data to output (i.e. the XML close can be on the same line as the open)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool XMLEmpty() const {return false;}

	/*--------------------------------------------------------------------------------*/
	/** Serialize (or prepare to serialize) additional parts of this object
	 *
	 * @param dst destination buffer or NULL to calculate required buffer size
	 * @param len offset into data to store data, modified by function
	 *
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SerializeEx(uint8_t *dst, uint_t& len) const;

protected:
	std::vector<ADMAudioBlockFormat *> blockformatrefs;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMAudioTrackFormat : public ADMObject, public ADMLevelObject, public ADMTimeObject {
public:
	/*--------------------------------------------------------------------------------*/
	/** ADM AudioTrackFormat object
	 *
	 * @param _owner an instance of ADMData that this object should belong to
	 * @param _id unique ID for this object (specified as part of the ADM)
	 * @param _name optional human-friendly name of the object
	 *
	 * @note type passed to base constructor is fixed by static member variable Type 
	 */
	/*--------------------------------------------------------------------------------*/
	ADMAudioTrackFormat(ADMData& _owner, const std::string& _id, const std::string& _name) : ADMObject(_owner, _id, _name),
																							 ADMLevelObject(),
																							 ADMTimeObject() {Register();}

	/*--------------------------------------------------------------------------------*/
	/** Return textual type name of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const std::string& GetType() const {return Type;}

	/*--------------------------------------------------------------------------------*/
	/** Returns textual reference type name of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const std::string& GetReference() const {return Reference;}

	/*--------------------------------------------------------------------------------*/
	/** Return serial data type of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual SerialDataType_t GetSerialDataType() const {return SerialDataType_TrackFormat;}

	/*--------------------------------------------------------------------------------*/
	/** Set internal variables from values added to internal list (e.g. from XML)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SetValues();

	/*--------------------------------------------------------------------------------*/
	/** Set and Get formatLabel
	 */
	/*--------------------------------------------------------------------------------*/
	void SetFormatLabel(const std::string& str) {formatLabel = str;}
	const std::string& GetFormatLabel() const {return formatLabel;}

	/*--------------------------------------------------------------------------------*/
	/** Set and Get formatDefinition
	 */
	/*--------------------------------------------------------------------------------*/
	void SetFormatDefinition(const std::string& str) {formatDefinition = str;}
	const std::string& GetFormatDefinition() const {return formatDefinition;}

	/*--------------------------------------------------------------------------------*/
	/** Add reference to AudioStreamFormat object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool Add(ADMAudioStreamFormat *obj);
	/*--------------------------------------------------------------------------------*/
	/** Return list of AudioStreamFormats
	 */
	/*--------------------------------------------------------------------------------*/
	const std::vector<ADMAudioStreamFormat *>& GetStreamFormatRefs() const {return streamformatrefs;}

	/*--------------------------------------------------------------------------------*/
	/** Update time limits
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void UpdateLimits();

	/*--------------------------------------------------------------------------------*/
	/** Output additional XML data for this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLData(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, std::vector<const ADMObject *>& reflist) const;

	// static type name
	static const std::string Type;

	// static type reference name
	static const std::string Reference;

protected:
	/*--------------------------------------------------------------------------------*/
	/** Dump additional information about this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void DumpEx(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const;

	/*--------------------------------------------------------------------------------*/
	/** Output additional XML attributes for this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLAttributes(std::string& str) const;

	/*--------------------------------------------------------------------------------*/
	/** Return true if there is no additional XML data to output (i.e. the XML close can be on the same line as the open)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool XMLEmpty() const {return (ADMObject::XMLEmpty() && (streamformatrefs.size() == 0));}

	/*--------------------------------------------------------------------------------*/
	/** Serialize (or prepare to serialize) additional parts of this object
	 *
	 * @param dst destination buffer or NULL to calculate required buffer size
	 * @param len offset into data to store data, modified by function
	 *
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SerializeEx(uint8_t *dst, uint_t& len) const;

protected:
	std::string formatLabel;
	std::string formatDefinition;
	std::vector<ADMAudioStreamFormat *> streamformatrefs;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMAudioBlockFormat : public ADMObject, public ADMLevelObject {
public:
	/*--------------------------------------------------------------------------------*/
	/** ADM AudioBlockFormat object
	 *
	 * @param _owner an instance of ADMData that this object should belong to
	 * @param _id unique ID for this object (specified as part of the ADM)
	 * @param _name optional human-friendly name of the object
	 *
	 * @note type passed to base constructor is fixed by static member variable Type 
	 */
	/*--------------------------------------------------------------------------------*/
	ADMAudioBlockFormat(ADMData& _owner, const std::string& _id, const std::string& _name) : ADMObject(_owner, _id, _name),
																							 ADMLevelObject(),
																							 rtime(0),
																							 duration(0),
																							 position(),
																							 supplement() {Register();}

	/*--------------------------------------------------------------------------------*/
	/** Return textual type name of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const std::string& GetType() const {return Type;}

	/*--------------------------------------------------------------------------------*/
	/** Returns textual reference type name of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const std::string& GetReference() const {return Reference;}

	/*--------------------------------------------------------------------------------*/
	/** Return serial data type of this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual SerialDataType_t GetSerialDataType() const {return SerialDataType_BlockFormat;}

	/*--------------------------------------------------------------------------------*/
	/** Set internal variables from values added to internal list (e.g. from XML)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SetValues();

	/*--------------------------------------------------------------------------------*/
	/** Set and Get rtime
	 */
	/*--------------------------------------------------------------------------------*/
	void     SetRTime(uint64_t t) {rtime = t;}
	uint64_t GetRTime() const {return rtime;}

	/*--------------------------------------------------------------------------------*/
	/** Set and Get duration
	 */
	/*--------------------------------------------------------------------------------*/
	void     SetDuration(uint64_t t) {duration = t;}
	uint64_t GetDuration() const {return duration;}

	/*--------------------------------------------------------------------------------*/
	/** Return block start time
	 */
	/*--------------------------------------------------------------------------------*/
	uint64_t GetStartTime() const {return rtime;}

	/*--------------------------------------------------------------------------------*/
	/** Return block end time
	 */
	/*--------------------------------------------------------------------------------*/
	uint64_t GetEndTime() const {return rtime + duration;}

	/*--------------------------------------------------------------------------------*/
	/** Return (modifiable) physical position of this object
	 */
	/*--------------------------------------------------------------------------------*/
	Position& GetPosition() {return position;}
	/*--------------------------------------------------------------------------------*/
	/** Return (non-modifiable) physical position of this object
	 */
	/*--------------------------------------------------------------------------------*/
	const Position& GetPosition() const {return position;}

	/*--------------------------------------------------------------------------------*/
	/** Return (modifiable) supplementary information of this object
	 */
	/*--------------------------------------------------------------------------------*/
	ParameterSet& GetPositionSupplement() {return supplement;}
	/*--------------------------------------------------------------------------------*/
	/** Return (non-modifiable) supplementary information of this object
	 */
	/*--------------------------------------------------------------------------------*/
	const ParameterSet& GetPositionSupplement() const {return supplement;}

	/*--------------------------------------------------------------------------------*/
	/** Output additional XML data for this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLData(std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level, std::vector<const ADMObject *>& reflist) const;

	static bool Compare(const ADMAudioBlockFormat *block1, const ADMAudioBlockFormat *block2) {
		return (block1->rtime < block2->rtime);
	}

	// static type name
	static const std::string Type;

	// static type reference name
	static const std::string Reference;

protected:
	/*--------------------------------------------------------------------------------*/
	/** Dump additional information about this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void DumpEx(std::map<const ADMObject *,bool>& handledmap, std::string& str, const std::string& indent, const std::string& eol, uint_t ind_level) const;

	/*--------------------------------------------------------------------------------*/
	/** Output additional XML attributes for this object
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void XMLAttributes(std::string& str) const;

	/*--------------------------------------------------------------------------------*/
	/** Return true if there is no additional XML data to output (i.e. the XML close can be on the same line as the open)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool XMLEmpty() const {return false;}

	/*--------------------------------------------------------------------------------*/
	/** Serialize (or prepare to serialize) additional parts of this object
	 *
	 * @param dst destination buffer or NULL to calculate required buffer size
	 * @param len offset into data to store data, modified by function
	 *
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void SerializeEx(uint8_t *dst, uint_t& len) const;

protected:
	uint64_t 	 rtime;
	uint64_t 	 duration;
	Position 	 position;
	ParameterSet supplement;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMTrackCursor : public PositionCursor {
public:
	ADMTrackCursor(const ADMAudioTrack *itrack = NULL);
	ADMTrackCursor(const ADMTrackCursor& obj);
	virtual ~ADMTrackCursor();

	void Setup(const ADMAudioTrack *itrack);

	ADMTrackCursor& operator = (const ADMAudioTrack *itrack) {Setup(itrack); return *this;}
	ADMTrackCursor& operator = (const ADMTrackCursor& obj);

	/*--------------------------------------------------------------------------------*/
	/** Get position at specified time (ns)
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool Seek(uint64_t t);

	/*--------------------------------------------------------------------------------*/
	/** Return position at current time
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const Position *GetPosition() const;

	/*--------------------------------------------------------------------------------*/
	/** Return supplementary information
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const ParameterSet *GetPositionSupplement() const;

protected:
	const ADMAudioTrack						 *track;
	const std::vector<ADMAudioBlockFormat *> *blockformats;
	uint_t blockindex;
};

BBC_AUDIOTOOLBOX_END

#endif
