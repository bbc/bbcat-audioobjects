#ifndef __ADM_OBJECTS__
#define __ADM_OBJECTS__

#include <stdarg.h>

#include <string>
#include <vector>
#include <map>

#include <bbcat-base/misc.h>
#include <bbcat-base/PositionCursor.h>

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
class ADMObject
{
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
   *
   * @param id new ID
   *
   * @note setting the ID updates the map held within the ADMData object
   */
  /*--------------------------------------------------------------------------------*/
  void SetID(const std::string& _id);
  const std::string& GetID()   const {return id;}

  /*--------------------------------------------------------------------------------*/
  /** Return ID prefix string
   */
  /*--------------------------------------------------------------------------------*/
  virtual const std::string& GetIDPrefix() const = 0;

  /*--------------------------------------------------------------------------------*/
  /** Set and Get object name (human-friendly)
   */
  /*--------------------------------------------------------------------------------*/
  void SetName(const std::string& _name) {name= _name;}
  const std::string& GetName() const {return name;}

  /*--------------------------------------------------------------------------------*/
  /** Get map entry
   */
  /*--------------------------------------------------------------------------------*/
  std::string GetMapEntryID() const {return GetType() + "/" + id;}

  /*--------------------------------------------------------------------------------*/
  /** Set and Get object typeLabel
   *
   * @note if type is a recognized typeLabel, typeDefinition will automatically be set!
   */
  /*--------------------------------------------------------------------------------*/
  void   SetTypeLabel(uint_t type);
  uint_t GetTypeLabel() const {return typeLabel;}

  /*--------------------------------------------------------------------------------*/
  /** Set and Get object typeDefinition
   */
  /*--------------------------------------------------------------------------------*/
  void SetTypeDefinition(const std::string& str) {typeDefinition = str;}
  const std::string& GetTypeDefinition() const {return typeDefinition;}

  /*--------------------------------------------------------------------------------*/
  /** Return owner of this object
   */
  /*--------------------------------------------------------------------------------*/
  const ADMData& GetOwner() const {return owner;}
  ADMData& GetOwner() {return owner;}

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
  typedef struct
  {
    bool        attr;       // true if this value is a simple XML attribute
    std::string name;       // value name
    std::string value;      // value value
    ADMATTRS    attrs;      // additional attributes (if attr == false)
  } ADMVALUE;
  typedef std::vector<ADMVALUE> ADMVALUES;

  /*--------------------------------------------------------------------------------*/
  /** Set ADMVALUE object as a simple XML attribute (name/value pair)
   */
  /*--------------------------------------------------------------------------------*/
  static void SetAttribute(ADMVALUE& obj, const std::string& name, const std::string& value) {SetValue(obj, name, value, true);}
  static void SetAttribute(ADMVALUE& obj, const std::string& name, uint_t             value, const char *format = "%u")     {SetValue(obj, name, value, true, format);}
  static void SetAttribute(ADMVALUE& obj, const std::string& name, uint64_t           value) {SetValue(obj, name, value, true);}
  static void SetAttribute(ADMVALUE& obj, const std::string& name, double             value, const char *format = "%0.6lf") {SetValue(obj, name, value, true, format);}

  /*--------------------------------------------------------------------------------*/
  /** Set ADMVALUE object as a XML value (name/value pair) with optional attributes
   */
  /*--------------------------------------------------------------------------------*/
  static void SetValue(ADMVALUE& obj, const std::string& name, const std::string& value, bool attr = false);
  static void SetValue(ADMVALUE& obj, const std::string& name, uint_t             value, bool attr = false, const char *format = "%u");
  static void SetValue(ADMVALUE& obj, const std::string& name, uint64_t           value, bool attr = false);
  static void SetValue(ADMVALUE& obj, const std::string& name, double             value, bool attr = false, const char *format = "%0.6lf");

  /*--------------------------------------------------------------------------------*/
  /** Set attribute of ADMVALUE value object (initialised above)
   */
  /*--------------------------------------------------------------------------------*/
  static void SetValueAttribute(ADMVALUE& obj, const std::string& name, const std::string& value);
  static void SetValueAttribute(ADMVALUE& obj, const std::string& name, uint_t             value, const char *format = "%u");
  static void SetValueAttribute(ADMVALUE& obj, const std::string& name, uint64_t           value);
  static void SetValueAttribute(ADMVALUE& obj, const std::string& name, double             value, const char *format = "%0.6lf");

  /*--------------------------------------------------------------------------------*/
  /** Add a value to the internal list
   */
  /*--------------------------------------------------------------------------------*/
  virtual void AddValue(const ADMVALUE& value);

  /*--------------------------------------------------------------------------------*/
  /** Return list of values/attributes from internal variables and list of referenced objects
   *
   * @param objvalues list to be populated with ADMVALUE's holding object attributes and values
   * @param objects list to be populdated with referenced or contained objects
   * @param full true to generate complete list including values that do not appear in the XML
   */
  /*--------------------------------------------------------------------------------*/
  typedef struct {
    const ADMObject *obj;
    bool            genref;     // generate reference to object from this object
    bool            gendata;    // output object within this object
  } REFERENCEDOBJECT;
  virtual void GetValuesAndReferences(ADMVALUES& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full = false) const;

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
  /** Generate a textual list of references 
   *
   * @param str string to be modified
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GenerateReferenceList(std::string& str) const {UNUSED_PARAMETER(str);}

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
  
  enum {
    TypeLabel_DirectSpeakers = 1,
    TypeLabel_Matrix,
    TypeLabel_Objects,
    TypeLabel_HOA,
    TypeLabel_Binaural,

    TypeLabel_Custom = 0x1000,
  };

  static void SetTypeDefinition(uint_t type, const std::string& definition)     {typeLabelMap[type]     = definition;}
  static void SetFormatDefinition(uint_t format, const std::string& definition) {formatLabelMap[format] = definition;}

protected:
  friend class ADMData;

  /*--------------------------------------------------------------------------------*/
  /** Register this object with the owner
   */
  /*--------------------------------------------------------------------------------*/
  void Register();

  /*--------------------------------------------------------------------------------*/
  /** Set updated ID (called from ADMData object only)
   */
  /*--------------------------------------------------------------------------------*/
  void SetUpdatedID(const std::string& _id) {id = _id;}

  /*--------------------------------------------------------------------------------*/
  /** Update object's ID
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdateID();

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
   *
   * @param value address of value to be erased
   *
   * @note value passed MUST be the address of the desired item
   */
  /*--------------------------------------------------------------------------------*/
  void EraseValue(const ADMVALUE *value);

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
  bool SetValue(std::string& res, const std::string& name);
  bool SetValue(double& res, const std::string& name);
  bool SetValue(uint_t& res, const std::string& name, bool hex = false);
  bool SetValue(ulong_t& res, const std::string& name, bool hex = false);
  bool SetValue(sint_t& res, const std::string& name, bool hex = false);
  bool SetValue(slong_t& res, const std::string& name, bool hex = false);
  bool SetValue(bool& res, const std::string& name);
  bool SetValueTime(uint64_t& res, const std::string& name);

  /*--------------------------------------------------------------------------------*/
  /** Prototypes for adding references of different kinds.  Any unimplemented handlers will result in references of that kind to be rejected
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool Add(ADMAudioContent       *obj) {UNUSED_PARAMETER(obj); return false;}
  virtual bool Add(ADMAudioObject        *obj) {UNUSED_PARAMETER(obj); return false;}
  virtual bool Add(ADMAudioTrack         *obj) {UNUSED_PARAMETER(obj); return false;}
  virtual bool Add(ADMAudioPackFormat    *obj) {UNUSED_PARAMETER(obj); return false;}
  virtual bool Add(ADMAudioStreamFormat  *obj) {UNUSED_PARAMETER(obj); return false;}
  virtual bool Add(ADMAudioChannelFormat *obj) {UNUSED_PARAMETER(obj); return false;}
  virtual bool Add(ADMAudioTrackFormat   *obj) {UNUSED_PARAMETER(obj); return false;}
  virtual bool Add(ADMAudioBlockFormat   *obj) {UNUSED_PARAMETER(obj); return false;}

  /*--------------------------------------------------------------------------------*/
  /** Generate a textual reference 
   *
   * @param str string to be modified
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GenerateObjectReference(std::string& str) const {Printf(str, "%s:%s", GetType().c_str(), GetName().c_str());}

  /*--------------------------------------------------------------------------------*/
  /** Generate a textual reference 
   *
   * @param str string to be modified
   * @param obj object to link to
   *
   */
  /*--------------------------------------------------------------------------------*/
  void GenerateReference(std::string& str, const ADMObject *obj) const {GenerateObjectReference(str); str += "->"; obj->GenerateObjectReference(str); str += "\n";}

protected:
  ADMData&    owner;
  std::string id;
  std::string name;
  uint_t      typeLabel;
  std::string typeDefinition;
  ADMVALUES   values;

  static std::map<uint_t,std::string> typeLabelMap;
  static std::map<uint_t,std::string> formatLabelMap;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMLevelObject
{
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

class ADMAudioProgramme : public ADMObject, public ADMLevelObject
{
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
  /** Return ID prefix string
   */
  /*--------------------------------------------------------------------------------*/
  virtual const std::string& GetIDPrefix() const {return IDPrefix;}

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
  /** Return list of values/attributes from internal variables and list of referenced objects
   *
   * @param objvalues list to be populated with ADMVALUE's holding object attributes and values
   * @param objects list to be populdated with referenced or contained objects
   * @param full true to generate complete list including values that do not appear in the XML
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetValuesAndReferences(ADMVALUES& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full = false) const;

  /*--------------------------------------------------------------------------------*/
  /** Generate a textual list of references 
   *
   * @param str string to be modified
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GenerateReferenceList(std::string& str) const;

  // static type name
  static const std::string Type;

  // static type reference name
  static const std::string Reference;

  // static type id prefix
  static const std::string IDPrefix;

protected:
  std::vector<ADMAudioContent *> contentrefs;
  std::string                    language;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMAudioContent : public ADMObject, public ADMLevelObject
{
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
  /** Return ID prefix string
   */
  /*--------------------------------------------------------------------------------*/
  virtual const std::string& GetIDPrefix() const {return IDPrefix;}

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
  /** Return list of values/attributes from internal variables and list of referenced objects
   *
   * @param objvalues list to be populated with ADMVALUE's holding object attributes and values
   * @param objects list to be populdated with referenced or contained objects
   * @param full true to generate complete list including values that do not appear in the XML
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetValuesAndReferences(ADMVALUES& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full = false) const;

  /*--------------------------------------------------------------------------------*/
  /** Generate a textual list of references 
   *
   * @param str string to be modified
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GenerateReferenceList(std::string& str) const;

  // static type name
  static const std::string Type;

  // static type reference name
  static const std::string Reference;

  // static type id prefix
  static const std::string IDPrefix;

protected:
  std::string                   language;
  std::vector<ADMAudioObject *> objectrefs;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMAudioObject : public ADMObject, public ADMLevelObject
{
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
  /** Return ID prefix string
   */
  /*--------------------------------------------------------------------------------*/
  virtual const std::string& GetIDPrefix() const {return IDPrefix;}

  /*--------------------------------------------------------------------------------*/
  /** Set internal variables from values added to internal list (e.g. from XML)
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetValues();

  /*--------------------------------------------------------------------------------*/
  /** Set and Get startTime
   */
  /*--------------------------------------------------------------------------------*/
  void     SetStartTime(uint64_t t) {startTime = t;}
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
  /** Return list of values/attributes from internal variables and list of referenced objects
   *
   * @param objvalues list to be populated with ADMVALUE's holding object attributes and values
   * @param objects list to be populdated with referenced or contained objects
   * @param full true to generate complete list including values that do not appear in the XML
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetValuesAndReferences(ADMVALUES& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full = false) const;

  static bool Compare(const ADMAudioObject *obj1, const ADMAudioObject *obj2)
  {
    return (obj1->GetStartTime() < obj2->GetStartTime());
  }

  /*--------------------------------------------------------------------------------*/
  /** Generate a textual list of references 
   *
   * @param str string to be modified
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GenerateReferenceList(std::string& str) const;

  // static type name
  static const std::string Type;

  // static type reference name
  static const std::string Reference;

  // static type id prefix
  static const std::string IDPrefix;

protected:
  std::vector<ADMAudioObject     *> objectrefs;
  std::vector<ADMAudioPackFormat *> packformatrefs;
  std::vector<ADMAudioTrack      *> trackrefs;
  uint64_t                          startTime;
  uint64_t                          duration;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMAudioTrack : public ADMObject, public ADMLevelObject
{
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
  ADMAudioTrack(ADMData& _owner, const std::string& _id, const std::string& _name) :
    ADMObject(_owner, _id, _name),
    ADMLevelObject(),
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
  /** Return ID prefix string
   */
  /*--------------------------------------------------------------------------------*/
  virtual const std::string& GetIDPrefix() const {return IDPrefix;}

  /*--------------------------------------------------------------------------------*/
  /** Set internal variables from values added to internal list (e.g. from XML)
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetValues();

  /*--------------------------------------------------------------------------------*/
  /** Set and Get track index
   *
   * @note trackNum is stored 0- based but is 1- based in CHNA chunk
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
  /** Return list of values/attributes from internal variables and list of referenced objects
   *
   * @param objvalues list to be populated with ADMVALUE's holding object attributes and values
   * @param objects list to be populdated with referenced or contained objects
   * @param full true to generate complete list including values that do not appear in the XML
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetValuesAndReferences(ADMVALUES& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full = false) const;

  static bool Compare(const ADMAudioTrack *track1, const ADMAudioTrack *track2)
  {
    return (track1->trackNum < track2->trackNum);
  }

  /*--------------------------------------------------------------------------------*/
  /** Generate a textual list of references 
   *
   * @param str string to be modified
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GenerateReferenceList(std::string& str) const;

  // static type name
  static const std::string Type;

  // static type reference name
  static const std::string Reference;

  // static type id prefix
  static const std::string IDPrefix;

protected:
  /*--------------------------------------------------------------------------------*/
  /** Generate a textual reference 
   *
   * @param str string to be modified
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GenerateObjectReference(std::string& str) const {Printf(str, "%s:%u", GetType().c_str(), trackNum);}

protected:
  uint_t                             trackNum;
  uint32_t                           sampleRate;
  uint_t                             bitDepth;
  std::vector<ADMAudioTrackFormat *> trackformatrefs;
  std::vector<ADMAudioPackFormat *>  packformatrefs;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMAudioPackFormat : public ADMObject
{
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
  ADMAudioPackFormat(ADMData& _owner, const std::string& _id, const std::string& _name) : ADMObject(_owner, _id, _name) {Register();}

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
  /** Return ID prefix string
   */
  /*--------------------------------------------------------------------------------*/
  virtual const std::string& GetIDPrefix() const {return IDPrefix;}

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
  /** Return list of values/attributes from internal variables and list of referenced objects
   *
   * @param objvalues list to be populated with ADMVALUE's holding object attributes and values
   * @param objects list to be populdated with referenced or contained objects
   * @param full true to generate complete list including values that do not appear in the XML
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetValuesAndReferences(ADMVALUES& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full = false) const;

  /*--------------------------------------------------------------------------------*/
  /** Generate a textual list of references 
   *
   * @param str string to be modified
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GenerateReferenceList(std::string& str) const;

  // static type name
  static const std::string Type;

  // static type reference name
  static const std::string Reference;

  // static type id prefix
  static const std::string IDPrefix;

protected:
  /*--------------------------------------------------------------------------------*/
  /** Update object's ID
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdateID();

protected:
  std::vector<ADMAudioChannelFormat *> channelformatrefs;
  std::vector<ADMAudioPackFormat    *> packformatrefs;
  std::string typeDefinition;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMAudioStreamFormat : public ADMObject
{
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
                                                                                            formatLabel(0) {Register();}

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
  /** Return ID prefix string
   */
  /*--------------------------------------------------------------------------------*/
  virtual const std::string& GetIDPrefix() const {return IDPrefix;}

  /*--------------------------------------------------------------------------------*/
  /** Set and Get formatLabel
   */
  /*--------------------------------------------------------------------------------*/
  void SetFormatLabel(uint_t format) {formatLabel = format; UpdateID();}
  uint_t GetFormatLabel() const {return formatLabel;}

  /*--------------------------------------------------------------------------------*/
  /** Set and Get formatDefinition
   */
  /*--------------------------------------------------------------------------------*/
  void SetFormatDefinition(const std::string& str) {formatDefinition = str;}
  const std::string& GetFormatDefinition() const {return formatDefinition;}

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
  /** Return list of values/attributes from internal variables and list of referenced objects
   *
   * @param objvalues list to be populated with ADMVALUE's holding object attributes and values
   * @param objects list to be populdated with referenced or contained objects
   * @param full true to generate complete list including values that do not appear in the XML
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetValuesAndReferences(ADMVALUES& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full = false) const;

  /*--------------------------------------------------------------------------------*/
  /** Generate a textual list of references 
   *
   * @param str string to be modified
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GenerateReferenceList(std::string& str) const;

  // static type name
  static const std::string Type;

  // static type reference name
  static const std::string Reference;

  // static type id prefix
  static const std::string IDPrefix;

protected:
  /*--------------------------------------------------------------------------------*/
  /** Update object's ID
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdateID();

protected:
  uint_t                               formatLabel;
  std::string                          formatDefinition;
  std::vector<ADMAudioChannelFormat *> channelformatrefs;
  std::vector<ADMAudioTrackFormat *>   trackformatrefs;
  std::vector<ADMAudioPackFormat *>    packformatrefs;
};

class ADMAudioChannelFormat : public ADMObject
{
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
  ADMAudioChannelFormat(ADMData& _owner, const std::string& _id, const std::string& _name) : ADMObject(_owner, _id, _name) {Register();}

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
  /** Return ID prefix string
   */
  /*--------------------------------------------------------------------------------*/
  virtual const std::string& GetIDPrefix() const {return IDPrefix;}

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
  /** Return list of AudioBlockFormats
   */
  /*--------------------------------------------------------------------------------*/
  std::vector<ADMAudioBlockFormat *>& GetBlockFormatRefs() {return blockformatrefs;}

  /*--------------------------------------------------------------------------------*/
  /** Return list of values/attributes from internal variables and list of referenced objects
   *
   * @param objvalues list to be populated with ADMVALUE's holding object attributes and values
   * @param objects list to be populdated with referenced or contained objects
   * @param full true to generate complete list including values that do not appear in the XML
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetValuesAndReferences(ADMVALUES& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full = false) const;

  /*--------------------------------------------------------------------------------*/
  /** Generate a textual list of references 
   *
   * @param str string to be modified
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GenerateReferenceList(std::string& str) const;

  // static type name
  static const std::string Type;

  // static type reference name
  static const std::string Reference;

  // static type id prefix
  static const std::string IDPrefix;

protected:
  /*--------------------------------------------------------------------------------*/
  /** Update object's ID
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdateID();

protected:
  std::vector<ADMAudioBlockFormat *> blockformatrefs;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMAudioTrackFormat : public ADMObject, public ADMLevelObject
{
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
                                                                                           formatLabel(0) {Register();}

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
  /** Return ID prefix string
   */
  /*--------------------------------------------------------------------------------*/
  virtual const std::string& GetIDPrefix() const {return IDPrefix;}

  /*--------------------------------------------------------------------------------*/
  /** Set internal variables from values added to internal list (e.g. from XML)
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetValues();

  /*--------------------------------------------------------------------------------*/
  /** Set and Get formatLabel
   */
  /*--------------------------------------------------------------------------------*/
  void SetFormatLabel(uint_t format) {formatLabel = format; UpdateID();}
  uint_t GetFormatLabel() const {return formatLabel;}

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
  /** Return list of values/attributes from internal variables and list of referenced objects
   *
   * @param objvalues list to be populated with ADMVALUE's holding object attributes and values
   * @param objects list to be populdated with referenced or contained objects
   * @param full true to generate complete list including values that do not appear in the XML
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetValuesAndReferences(ADMVALUES& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full = false) const;

  /*--------------------------------------------------------------------------------*/
  /** Generate a textual list of references 
   *
   * @param str string to be modified
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GenerateReferenceList(std::string& str) const;

  // static type name
  static const std::string Type;

  // static type reference name
  static const std::string Reference;

  // static type id prefix
  static const std::string IDPrefix;

protected:
  /*--------------------------------------------------------------------------------*/
  /** Update object's ID
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdateID();

protected:
  uint_t      formatLabel;
  std::string formatDefinition;
  std::vector<ADMAudioStreamFormat *> streamformatrefs;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMAudioBlockFormat : public ADMObject, public ADMLevelObject
{
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
  ADMAudioBlockFormat(ADMData& _owner, const std::string& _id, const std::string& _name) :
    ADMObject(_owner, _id, _name),
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
  /** Return ID prefix string
   */
  /*--------------------------------------------------------------------------------*/
  virtual const std::string& GetIDPrefix() const {return IDPrefix;}

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
  /** Set position for this blockformat
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetPosition(const Position& pos, const ParameterSet *supplement = NULL);

  /*--------------------------------------------------------------------------------*/
  /** Return list of values/attributes from internal variables and list of referenced objects
   *
   * @param objvalues list to be populated with ADMVALUE's holding object attributes and values
   * @param objects list to be populdated with referenced or contained objects
   * @param full true to generate complete list including values that do not appear in the XML
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetValuesAndReferences(ADMVALUES& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full = false) const;

  static bool Compare(const ADMAudioBlockFormat *block1, const ADMAudioBlockFormat *block2)
  {
    return (block1->rtime < block2->rtime);
  }

  /*--------------------------------------------------------------------------------*/
  /** Generate a textual list of references 
   *
   * @param str string to be modified
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GenerateReferenceList(std::string& str) const;

  // static type name
  static const std::string Type;

  // static type reference name
  static const std::string Reference;

  // static type id prefix
  static const std::string IDPrefix;

protected:
  uint64_t     rtime;
  uint64_t     duration;
  Position     position;
  ParameterSet supplement;
};

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Implementation of PositionCursor using ADMBlockFormat objects as position source
 */
/*--------------------------------------------------------------------------------*/
class ADMTrackCursor : public PositionCursor
{
public:
  ADMTrackCursor(uint_t _channel);
  ADMTrackCursor(const ADMTrackCursor& obj);
  virtual ~ADMTrackCursor();

  /*--------------------------------------------------------------------------------*/
  /** Add audio object to this object
   *
   * @return true if object added, false if object ignored
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool Add(const ADMAudioObject *object, bool sort = true);

  /*--------------------------------------------------------------------------------*/
  /** Add audio objects to this object
   */
  /*--------------------------------------------------------------------------------*/
  bool Add(const ADMAudioObject *objects[], uint_t n);

  /*--------------------------------------------------------------------------------*/
  /** Add audio objects to this object
   */
  /*--------------------------------------------------------------------------------*/
  bool Add(const std::vector<const ADMAudioObject *>& objects);

  /*--------------------------------------------------------------------------------*/
  /** Return cursor start time in ns
   */
  /*--------------------------------------------------------------------------------*/
  virtual uint64_t GetStartTime() const;

  /*--------------------------------------------------------------------------------*/
  /** Return cursor end time in ns
   */
  /*--------------------------------------------------------------------------------*/
  virtual uint64_t GetEndTime() const;

  /*--------------------------------------------------------------------------------*/
  /** Get position at specified time (ns)
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool Seek(uint64_t t);

  /*--------------------------------------------------------------------------------*/
  /** Return channel for this cursor
   */
  /*--------------------------------------------------------------------------------*/
  virtual uint_t GetChannel() const {return channel;}

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

  /*--------------------------------------------------------------------------------*/
  /** Set position for current time
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetPosition(const Position& pos, const ParameterSet *supplement = NULL);
  
  /*--------------------------------------------------------------------------------*/
  /** End position updates by marking the end of the last block
   */
  /*--------------------------------------------------------------------------------*/
  virtual void EndPositionChanges();

  /*--------------------------------------------------------------------------------*/
  /** Sort list of objects into time order
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Sort();

protected:
  /*--------------------------------------------------------------------------------*/
  /** Start a blockformat at t
   */
  /*--------------------------------------------------------------------------------*/
  virtual ADMAudioBlockFormat *StartBlockFormat(uint64_t t);

  /*--------------------------------------------------------------------------------*/
  /** End a blockformat at t that has previously been start
   */
  /*--------------------------------------------------------------------------------*/
  virtual void EndBlockFormat(uint64_t t);

protected:
  typedef struct {
    const ADMAudioObject  *object;              ///< ADMAudioObject object
    ADMAudioChannelFormat *channelformat;       ///< ADMAudioChannelFormat object holding block formats
  } AUDIOOBJECT;

  static bool Compare(const AUDIOOBJECT& obj1, const AUDIOOBJECT& obj2)
  {
    return (obj1.object->GetStartTime() < obj2.object->GetStartTime());
  }

protected:
  uint_t                   channel;
  std::vector<AUDIOOBJECT> objectlist;
  uint_t                   objectindex;
  uint_t                   blockindex;
  uint64_t                 currenttime;
  bool                     blockformatstarted;
};

BBC_AUDIOTOOLBOX_END

#endif
