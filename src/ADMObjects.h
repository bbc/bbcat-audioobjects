#ifndef __ADM_OBJECTS__
#define __ADM_OBJECTS__

#include <stdarg.h>

#include <string>
#include <vector>
#include <map>

#include <bbcat-base/misc.h>
#include <bbcat-base/ThreadLock.h>
#include <bbcat-control/AudioObjectCursor.h>
#include <bbcat-control/AudioObjectParameters.h>

#include "XMLValue.h"

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
   * @param start start index to find an ID from
   *
   * @note setting the ID updates the map held within the ADMData object
   * @note some types of ID start numbering at 0x1000, some at 0x0000
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetID(const std::string& _id, uint_t start = 0);
  virtual const std::string& GetID() const {return id;}

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
  virtual const std::string& GetName() const {return name;}

  /*--------------------------------------------------------------------------------*/
  /** Get map entry
   */
  /*--------------------------------------------------------------------------------*/
  std::string GetMapEntryID() const {return GetType() + "/" + id;}

  /*--------------------------------------------------------------------------------*/
  /** Set and Get object typeLabel
   *
   * @param type typeLabel index
   *
   * @note if type is a recognized typeLabel, typeDefinition will automatically be set!
   */
  /*--------------------------------------------------------------------------------*/
  void   SetTypeLabel(uint_t type);
  uint_t GetTypeLabel() const {return typeLabel;}

  /*--------------------------------------------------------------------------------*/
  /** Set and Get object typeDefinition
   *
   * @param type typeLabel index
   */
  /*--------------------------------------------------------------------------------*/
  void SetTypeDefinition(const std::string& str);
  const std::string& GetTypeDefinition() const {return typeDefinition;}

  /*--------------------------------------------------------------------------------*/
  /** Return owner of this object
   */
  /*--------------------------------------------------------------------------------*/
  const ADMData& GetOwner() const {return owner;}
  ADMData& GetOwner() {return owner;}

  /*--------------------------------------------------------------------------------*/
  /** Set and Get standard definition flag
   */
  /*--------------------------------------------------------------------------------*/
  void SetStandardDefinition(bool stddef = true) {standarddef = stddef;}
  bool IsStandardDefinition() const              {return standarddef;}

  /*--------------------------------------------------------------------------------*/
  /** Add a value to the internal list
   */
  /*--------------------------------------------------------------------------------*/
  void AddValue(const XMLValue& value) {values.AddValue(value);}

  /*--------------------------------------------------------------------------------*/
  /** Get XML value list
   */
  /*--------------------------------------------------------------------------------*/
  XMLValues& GetValues() {return values;}
  
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
  /** Return list of values/attributes from internal variables and list of referenced objects
   *
   * @param objvalues list to be populated with XMLValue's holding object attributes and values
   * @param objects list to be populdated with referenced or contained objects
   * @param full true to generate complete list including values that do not appear in the XML
   */
  /*--------------------------------------------------------------------------------*/
  typedef struct {
    ADMObject *obj;
    bool      genref;     // generate reference to object from this object
  } REFERENCEDOBJECT;
  virtual void GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full = false) const;

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
  typedef struct {
    std::string type;
    XMLValues   attrs;
    XMLValues   values;
  } CONTAINEDOBJECT;
  virtual uint_t GetContainedObjectCount() const {return 0;}
  virtual bool   GetContainedObject(uint_t n, CONTAINEDOBJECT& object) const
  {
    UNUSED_PARAMETER(n);
    UNUSED_PARAMETER(object);
    return false;
  }
    
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
  
  enum {
    TypeLabel_Unknown = 0,
    TypeLabel_DirectSpeakers,
    TypeLabel_Matrix,
    TypeLabel_Objects,
    TypeLabel_HOA,
    TypeLabel_Binaural,

    TypeLabel_Custom = 0x1000,
  };

  // absolute maximum time
  static const uint64_t MaxTime;

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
  virtual void GenerateObjectReference(std::string& str) const {Printf(str, "%s", GetID().c_str());}

  /*--------------------------------------------------------------------------------*/
  /** Generate a textual reference 
   *
   * @param str string to be modified
   * @param obj object to link to
   *
   */
  /*--------------------------------------------------------------------------------*/
  void GenerateReference(std::string& str, const ADMObject *obj) const {GenerateObjectReference(str); str += "->"; obj->GenerateObjectReference(str); str += "\n";}

  /*--------------------------------------------------------------------------------*/
  /** Update typeLabels of referenced objects
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdateRefTypeLabels() {}

  /*--------------------------------------------------------------------------------*/
  /** Update typeDefinitions of referenced objects
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdateRefTypeDefinitions() {}

  /*--------------------------------------------------------------------------------*/
  /** Update typeLabels of referenced objects
   */
  /*--------------------------------------------------------------------------------*/
  template<typename vectortype>
  void UpdateRefTypeLabelsList(std::vector<vectortype *>& list)
  {
    if (typeLabel != TypeLabel_Unknown)
    {
      typename std::vector<vectortype *>::iterator it;
      for (it = list.begin(); it != list.end(); ++it) (*it)->SetTypeLabel(typeLabel);
    }
  }

  /*--------------------------------------------------------------------------------*/
  /** Update typeDefinitions of referenced objects
   */
  /*--------------------------------------------------------------------------------*/
  template<class vectortype>
  void UpdateRefTypeDefinitionsList(std::vector<vectortype *>& list)
  {
    if (!typeDefinition.empty())
    {
      typename std::vector<vectortype *>::iterator it;
      for (it = list.begin(); it != list.end(); ++it) (*it)->SetTypeDefinition(typeDefinition);
    }
  }

  /*--------------------------------------------------------------------------------*/
  /** Set typeLabel and typeDefinition (if valid and ADM is not in pure mode) in supplied object
   */
  /*--------------------------------------------------------------------------------*/
  void SetTypeInfoInObject(ADMObject *obj) const;

protected:
  ADMData&     owner;
  std::string  id;
  std::string  name;
  uint_t       typeLabel;
  std::string  typeDefinition;
  XMLValues    values;
  bool         standarddef;

  static std::map<uint_t,std::string> typeLabelMap;
  static std::map<uint_t,std::string> formatLabelMap;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMAudioProgramme : public ADMObject
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
  ADMAudioProgramme(ADMData& _owner, const std::string& _id, const std::string& _name) : ADMObject(_owner, _id, _name) {Register();}
  virtual ~ADMAudioProgramme() {}
  
  typedef std::vector<ADMAudioProgramme *> LIST;

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
  /** Get ID (for AudioProgramme handling)
   */
  /*--------------------------------------------------------------------------------*/
  virtual const std::string& GetID() const {return ADMObject::GetID();}

  /*--------------------------------------------------------------------------------*/
  /** Get Name (for AudioProgramme handling)
   */
  /*--------------------------------------------------------------------------------*/
  virtual const std::string& GetName() const {return ADMObject::GetName();}

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
   * @param objvalues list to be populated with XMLValue's holding object attributes and values
   * @param objects list to be populdated with referenced or contained objects
   * @param full true to generate complete list including values that do not appear in the XML
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full = false) const;

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

class ADMAudioContent : public ADMObject
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
  ADMAudioContent(ADMData& _owner, const std::string& _id, const std::string& _name) : ADMObject(_owner, _id, _name) {Register();}
  virtual ~ADMAudioContent() {}

  typedef std::vector<ADMAudioContent *> LIST;
  
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
  /** Get ID (for AudioContent handling)
   */
  /*--------------------------------------------------------------------------------*/
  virtual const std::string& GetID() const {return ADMObject::GetID();}

  /*--------------------------------------------------------------------------------*/
  /** Get Name (for AudioContent handling)
   */
  /*--------------------------------------------------------------------------------*/
  virtual const std::string& GetName() const {return ADMObject::GetName();}

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
   * @param objvalues list to be populated with XMLValue's holding object attributes and values
   * @param objects list to be populdated with referenced or contained objects
   * @param full true to generate complete list including values that do not appear in the XML
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full = false) const;

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

class ADMAudioObject : public ADMObject, public AudioObject
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
  virtual ~ADMAudioObject() {}

  typedef std::vector<ADMAudioObject *> LIST;
  
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
  /** Get ID (for AudioObject handling)
   */
  /*--------------------------------------------------------------------------------*/
  virtual const std::string& GetID() const {return ADMObject::GetID();}

  /*--------------------------------------------------------------------------------*/
  /** Get Name (for AudioObject handling)
   */
  /*--------------------------------------------------------------------------------*/
  virtual const std::string& GetName() const {return ADMObject::GetName();}

  /*--------------------------------------------------------------------------------*/
  /** Set and Get startTime
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetStartTime(uint64_t t) {startTime = t; startTimeSet = true;}
  virtual uint64_t GetStartTime() const {return startTime;}
  bool StartTimeSet() const {return startTimeSet;}

  /*--------------------------------------------------------------------------------*/
  /** Set and Get duration
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetDuration(uint64_t t) {duration = t; durationSet = true;}
  virtual uint64_t GetDuration() const {return duration;}
  bool DurationSet() const {return durationSet;}

  /*--------------------------------------------------------------------------------*/
  /** Set and Get endTime
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetEndTime(uint64_t t) {duration = (t < ADMObject::MaxTime) ? subz<>(t, startTime) : 0;}
  virtual uint64_t GetEndTime() const {return duration ? addm<>(startTime, duration) : ADMObject::MaxTime;}

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
  /** Sort tracks
   */
  /*--------------------------------------------------------------------------------*/
  void SortTracks();
  
  /*--------------------------------------------------------------------------------*/
  /** Get start channel
   */
  /*--------------------------------------------------------------------------------*/
  uint_t GetStartChannel()         const {return starttrack;}
  void   SetStartChannel(uint_t channel) {starttrack = channel;}

  /*--------------------------------------------------------------------------------*/
  /** Get number of channels
   */
  /*--------------------------------------------------------------------------------*/
  uint_t GetChannelCount() const           {return trackcount;}
  void   SetChannelCount(uint_t nchannels) {trackcount = nchannels;}

  /*--------------------------------------------------------------------------------*/
  /** Set dialogue type
   */
  /*--------------------------------------------------------------------------------*/
  uint_t GetDialogue() const     {return dialogue;}
  void   SetDialogue(uint_t val) {dialogue = MIN(val, 2);}

  /*--------------------------------------------------------------------------------*/
  /** Set importance value
   */
  /*--------------------------------------------------------------------------------*/
  uint_t GetImportance() const     {return importance;}
  void   SetImportance(uint_t val) {importance = MIN(val, 10);}

  /*--------------------------------------------------------------------------------*/
  /** Set interact flag
   */
  /*--------------------------------------------------------------------------------*/
  bool   GetInteract() const   {return interact;}
  void   SetInteract(bool val) {interact = val;}

  /*--------------------------------------------------------------------------------*/
  /** Set disableducking flag
   */
  /*--------------------------------------------------------------------------------*/
  bool   GetDisableDucking() const   {return disableducking;}
  void   SetDisableDucking(bool val) {disableducking = val;}
  
  /*--------------------------------------------------------------------------------*/
  /** Process object parameters for rendering
   *
   * @param channel channel number (within object)
   * @param dst object parameters object
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ProcessObjectParameters(uint_t channel, AudioObjectParameters& dst);
  
  /*--------------------------------------------------------------------------------*/
  /** Use object parameters to set parameters within audio object
   *
   * @param src audio object parameters object
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdateAudioObject(const AudioObjectParameters& src);

  /*--------------------------------------------------------------------------------*/
  /** Return list of values/attributes from internal variables and list of referenced objects
   *
   * @param objvalues list to be populated with XMLValue's holding object attributes and values
   * @param objects list to be populdated with referenced or contained objects
   * @param full true to generate complete list including values that do not appear in the XML
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full = false) const;

  /*--------------------------------------------------------------------------------*/
  /** Get audioChannelFormat for a particular track
   */
  /*--------------------------------------------------------------------------------*/
  ADMAudioChannelFormat *GetChannelFormat(uint_t track) const;

  /*--------------------------------------------------------------------------------*/
  /** Get list of audioBlockFormats for a particular track
   */
  /*--------------------------------------------------------------------------------*/
  const std::vector<ADMAudioBlockFormat *> *GetBlockFormatList(uint_t track) const;
  
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

#if ENABLE_JSON
  /*--------------------------------------------------------------------------------*/
  /** Convert parameters to a JSON object
   *
   * ADM audio objects contain extra information for the JSON representation
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ToJSON(json_spirit::mObject& obj) const;
#endif

  // static type name
  static const std::string Type;

  // static type reference name
  static const std::string Reference;

  // static type id prefix
  static const std::string IDPrefix;
  
protected:
  std::vector<ADMAudioObject *>          objectrefs;
  std::vector<ADMAudioPackFormat *>      packformatrefs;
  std::vector<ADMAudioTrack *>           trackrefs;
  uint64_t                               startTime;
  uint64_t                               duration;
  uint_t                                 starttrack;
  uint_t                                 trackcount;
  uint_t                                 dialogue;
  uint_t                                 importance;
  bool                                   interact;
  bool                                   disableducking;
  bool                                   startTimeSet;
  bool                                   durationSet;
};

/*----------------------------------------------------------------------------------------------------*/

class ADMAudioTrack : public ADMObject
{
public:
  /*--------------------------------------------------------------------------------*/
  /** ADM AudioTrack object
   *
   * @param _owner an instance of ADMData that this object should belong to
   * @param _id unique ID for this object (specified as part of the ADM)
   *
   * @note type passed to base constructor is fixed by static member variable Type 
   */
  /*--------------------------------------------------------------------------------*/
  ADMAudioTrack(ADMData& _owner, const std::string& _id) :
    ADMObject(_owner, _id, ""),
    trackNum(0),
    sampleRate(0),
    bitDepth(0) {Register();}
  virtual ~ADMAudioTrack() {}
  
  typedef std::vector<ADMAudioTrack *> LIST;

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

  static bool Compare(const ADMAudioTrack *obj1, const ADMAudioTrack *obj2)
  {
    return (obj1->GetTrackNum() < obj2->GetTrackNum());
  }

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
   * @param objvalues list to be populated with XMLValue's holding object attributes and values
   * @param objects list to be populdated with referenced or contained objects
   * @param full true to generate complete list including values that do not appear in the XML
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full = false) const;

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
  /** Update typeLabels of referenced objects
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdateRefTypeLabels()
  {
    UpdateRefTypeLabelsList(trackformatrefs);
    UpdateRefTypeLabelsList(packformatrefs);
  }

  /*--------------------------------------------------------------------------------*/
  /** Update typeDefinitions of referenced objects
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdateRefTypeDefinitions()
  {
    UpdateRefTypeDefinitionsList(trackformatrefs);
    UpdateRefTypeDefinitionsList(packformatrefs);
  }

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
  virtual ~ADMAudioPackFormat() {}
  
  typedef std::vector<ADMAudioPackFormat *> LIST;

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
   * @param objvalues list to be populated with XMLValue's holding object attributes and values
   * @param objects list to be populdated with referenced or contained objects
   * @param full true to generate complete list including values that do not appear in the XML
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full = false) const;

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

  /*--------------------------------------------------------------------------------*/
  /** Update typeLabels of referenced objects
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdateRefTypeLabels()
  {
    UpdateRefTypeLabelsList(channelformatrefs);
    UpdateRefTypeLabelsList(packformatrefs);
  }

  /*--------------------------------------------------------------------------------*/
  /** Update typeDefinitions of referenced objects
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdateRefTypeDefinitions()
  {
    UpdateRefTypeDefinitionsList(channelformatrefs);
    UpdateRefTypeDefinitionsList(packformatrefs);
  }

protected:
  std::vector<ADMAudioChannelFormat *> channelformatrefs;
  std::vector<ADMAudioPackFormat *>    packformatrefs;
};

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** NOTE: an audioStreamFormat may be used by more than one audioObject!
 */
/*--------------------------------------------------------------------------------*/
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
  virtual ~ADMAudioStreamFormat() {}
  
  typedef std::vector<ADMAudioStreamFormat *> LIST;

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
  void SetFormatLabel(uint_t format) {formatLabel = format;}
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
   * @param objvalues list to be populated with XMLValue's holding object attributes and values
   * @param objects list to be populdated with referenced or contained objects
   * @param full true to generate complete list including values that do not appear in the XML
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full = false) const;

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

  /*--------------------------------------------------------------------------------*/
  /** Update typeLabels of referenced objects
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdateRefTypeLabels()
  {
    UpdateRefTypeLabelsList(channelformatrefs);
    UpdateRefTypeLabelsList(trackformatrefs);
    UpdateRefTypeLabelsList(packformatrefs);
  }

  /*--------------------------------------------------------------------------------*/
  /** Update typeDefinitions of referenced objects
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdateRefTypeDefinitions()
  {
    UpdateRefTypeDefinitionsList(channelformatrefs);
    UpdateRefTypeDefinitionsList(trackformatrefs);
    UpdateRefTypeDefinitionsList(packformatrefs);
  }

protected:
  uint_t                               formatLabel;
  std::string                          formatDefinition;
  std::vector<ADMAudioChannelFormat *> channelformatrefs;
  std::vector<ADMAudioTrackFormat *>   trackformatrefs;
  std::vector<ADMAudioPackFormat *>    packformatrefs;
};

/*--------------------------------------------------------------------------------*/
/** NOTE: an audioChannelFormat may be used by more than one audioObject!
 */
/*--------------------------------------------------------------------------------*/
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
  virtual ~ADMAudioChannelFormat();

  typedef std::vector<ADMAudioChannelFormat *> LIST;

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
   * @param objvalues list to be populated with XMLValue's holding object attributes and values
   * @param objects list to be populdated with referenced or contained objects
   * @param full true to generate complete list including values that do not appear in the XML
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full = false) const;

  /*--------------------------------------------------------------------------------*/
  /** Generate a textual list of references 
   *
   * @param str string to be modified
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GenerateReferenceList(std::string& str) const;

  /*--------------------------------------------------------------------------------*/
  /** Sort block formats in time order
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SortBlockFormats();
  
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
  virtual uint_t GetContainedObjectCount() const {return blockformatrefs.size();}
  virtual bool   GetContainedObject(uint_t n, CONTAINEDOBJECT& object) const;

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

/*--------------------------------------------------------------------------------*/
/** NOTE: an audioTrackFormat may be used by more than one audioObject!
 */
/*--------------------------------------------------------------------------------*/
class ADMAudioTrackFormat : public ADMObject
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
                                                                                           formatLabel(0) {Register();}
  virtual ~ADMAudioTrackFormat() {}
  
  typedef std::vector<ADMAudioTrackFormat *> LIST;

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
  void SetFormatLabel(uint_t format) {formatLabel = format;}
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
   * @param objvalues list to be populated with XMLValue's holding object attributes and values
   * @param objects list to be populdated with referenced or contained objects
   * @param full true to generate complete list including values that do not appear in the XML
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetValuesAndReferences(XMLValues& objvalues, std::vector<REFERENCEDOBJECT>& objects, bool full = false) const;

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

  /*--------------------------------------------------------------------------------*/
  /** Update typeLabels of referenced objects
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdateRefTypeLabels()
  {
    UpdateRefTypeLabelsList(streamformatrefs);
  }

  /*--------------------------------------------------------------------------------*/
  /** Update typeDefinitions of referenced objects
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdateRefTypeDefinitions()
  {
    UpdateRefTypeDefinitionsList(streamformatrefs);
  }

protected:
  uint_t                              formatLabel;
  std::string                         formatDefinition;
  std::vector<ADMAudioStreamFormat *> streamformatrefs;
};

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** audioBlockFormat
 *
 * NOTE: an audioBlockFormat may be used by more than one audioObject!
 */
/*--------------------------------------------------------------------------------*/
class ADMAudioBlockFormat   // NOT derived from ADMObject to save memory
{
public:
  /*--------------------------------------------------------------------------------*/
  /** ADM AudioBlockFormat object
   *
   * @note constructor code is in ADMObjects.cpp
   */
  /*--------------------------------------------------------------------------------*/
  ADMAudioBlockFormat();
  virtual ~ADMAudioBlockFormat() {}
  
  typedef std::vector<ADMAudioBlockFormat *> LIST;
  
  /*--------------------------------------------------------------------------------*/
  /** Return textual type name of this object
   */
  /*--------------------------------------------------------------------------------*/
  const std::string& GetType() const {return Type;}

  /*--------------------------------------------------------------------------------*/
  /** Returns textual reference type name of this object
   */
  /*--------------------------------------------------------------------------------*/
  const std::string& GetReference() const {return Reference;}

  /*--------------------------------------------------------------------------------*/
  /** Return ID prefix string
   */
  /*--------------------------------------------------------------------------------*/
  const std::string& GetIDPrefix() const {return IDPrefix;}

  /*--------------------------------------------------------------------------------*/
  /** Set internal variables from values added to internal list (e.g. from XML)
   */
  /*--------------------------------------------------------------------------------*/
  void SetValues(XMLValues& values);

  /*--------------------------------------------------------------------------------*/
  /** Set and Get rtime
   */
  /*--------------------------------------------------------------------------------*/
  void     SetRTime(uint64_t t) {rtime = t; rtimeSet = true;}
  uint64_t GetRTime() const {return rtime;}
  bool     RTimeSet() const {return rtimeSet;}

  /*--------------------------------------------------------------------------------*/
  /** Set and Get duration
   */
  /*--------------------------------------------------------------------------------*/
  void     SetDuration(uint64_t t) {duration = t; objparameters.SetDuration(t); durationSet = true;}
  uint64_t GetDuration() const {return duration;}
  bool     DurationSet() const {return durationSet;}

  /*--------------------------------------------------------------------------------*/
  /** Set and Get block start time (absolute)
   *
   * @param obj audio object this block is being used with
   */
  /*--------------------------------------------------------------------------------*/
  void SetStartTime(uint64_t t, const ADMAudioObject *obj = NULL) {SetRTime(obj ? subz<>(t, obj->GetStartTime()) : t);}
  uint64_t GetStartTime(const ADMAudioObject *obj = NULL) const {return obj ? addm<>(obj->GetStartTime(), rtime) : rtime;}

  /*--------------------------------------------------------------------------------*/
  /** Set and Get block end time (absolute)
   *
   * @param obj audio object this block is being used with
   */
  /*--------------------------------------------------------------------------------*/
  void SetEndTime(uint64_t t, const ADMAudioObject *obj = NULL) {SetDuration((t < ADMObject::MaxTime) ? subz<>(t, GetStartTime(obj)) : 0);}
  uint64_t GetEndTime(const ADMAudioObject *obj = NULL) const {return duration ? addm<>(GetStartTime(obj), duration) : ADMObject::MaxTime;}

  /*--------------------------------------------------------------------------------*/
  /** Return list of values/attributes from internal variables and list of referenced objects
   *
   * @param objattrs list to be populated with XMLValue's holding object attributes
   * @param objvalues list to be populated with XMLValue's holding object values
   * @param full true to generate complete list including values that do not appear in the XML
   */
  /*--------------------------------------------------------------------------------*/
  void GetValues(XMLValues& objattrs, XMLValues& objvalues, bool full = false) const;

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
  void GenerateReferenceList(std::string& str) const;

  /*--------------------------------------------------------------------------------*/
  /** Return modifiable object parameters object
   */
  /*--------------------------------------------------------------------------------*/
  AudioObjectParameters& GetObjectParameters() {return objparameters;}

  /*--------------------------------------------------------------------------------*/
  /** Return non-modifiable object parameters object
   */
  /*--------------------------------------------------------------------------------*/
  const AudioObjectParameters& GetObjectParameters() const {return objparameters;}

#if ENABLE_JSON
  /*--------------------------------------------------------------------------------*/
  /** Convert parameters to a JSON object
   */
  /*--------------------------------------------------------------------------------*/
  void ToJSON(json_spirit::mObject& obj) const;

  /*--------------------------------------------------------------------------------*/
  /** Convert parameters to a JSON object
   */
  /*--------------------------------------------------------------------------------*/
  json_spirit::mObject ToJSON() const {json_spirit::mObject obj; ToJSON(obj); return obj;}

  /*--------------------------------------------------------------------------------*/
  /** Operator overload
   */
  /*--------------------------------------------------------------------------------*/
  operator json_spirit::mObject() const {return ToJSON();}
 
  /*--------------------------------------------------------------------------------*/
  /** Convert parameters to a JSON string
   */
  /*--------------------------------------------------------------------------------*/
  std::string ToJSONString() const {return json_spirit::write(ToJSON(), json_spirit::pretty_print);}
#endif

  // static type name
  static const std::string Type;

  // static type reference name
  static const std::string Reference;

  // static type id prefix
  static const std::string IDPrefix;

protected:
  AudioObjectParameters       objparameters;
  uint64_t                    rtime;
  uint64_t                    duration;
  bool                        rtimeSet;
  bool                        durationSet;
};

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Implementation of AudioObjectCursor using ADMBlockFormat objects as source
 */
/*--------------------------------------------------------------------------------*/
class ADMTrackCursor : public AudioObjectCursor
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
  bool Add(const ADMAudioObject *objects[], uint_t n, bool sort = true);

  /*--------------------------------------------------------------------------------*/
  /** Add audio objects to this object
   */
  /*--------------------------------------------------------------------------------*/
  bool Add(const std::vector<const ADMAudioObject *>& objects, bool sort = true);

  /*--------------------------------------------------------------------------------*/
  /** Add audio objects to this object
   */
  /*--------------------------------------------------------------------------------*/
  bool Add(const std::vector<ADMAudioObject *>& objects, bool sort = true);

  /*--------------------------------------------------------------------------------*/
  /** Add audio objects to this object
   */
  /*--------------------------------------------------------------------------------*/
  bool Add(const std::vector<const ADMObject *>& objects, bool sort = true);

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
  /** Return audio object parameters at current time
   */
  /*--------------------------------------------------------------------------------*/
  virtual const ADMAudioBlockFormat *GetBlockFormat() const;

  /*--------------------------------------------------------------------------------*/
  /** Get current audio object
   */
  /*--------------------------------------------------------------------------------*/
  virtual AudioObject *GetAudioObject() const;

  /*--------------------------------------------------------------------------------*/
  /** Return audio object parameters at current time
   *
   * @return true if object parameters are valid and returned in currentparameters
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool GetObjectParameters(AudioObjectParameters& currentparameters) const;

  /*--------------------------------------------------------------------------------*/
  /** Set audio object parameters for current time
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetObjectParameters(const AudioObjectParameters& newparameters);
  
  /*--------------------------------------------------------------------------------*/
  /** End parameters updates by marking the end of the last block
   */
  /*--------------------------------------------------------------------------------*/
  virtual void EndChanges();

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
    const ADMAudioObject  *audioobject;         ///< ADMAudioObject object
    ADMAudioChannelFormat *channelformat;       ///< ADMAudioChannelFormat object holding block formats
  } AUDIOOBJECT;

  static bool Compare(const AUDIOOBJECT& obj1, const AUDIOOBJECT& obj2)
  {
    return (obj1.audioobject->GetStartTime() < obj2.audioobject->GetStartTime());
  }

protected:
  ThreadLockObject         tlock;
  uint_t                   channel;
  AudioObjectParameters    objparameters;
  std::vector<AUDIOOBJECT> objectlist;
  uint_t                   objectindex;
  uint_t                   blockindex;
  uint64_t                 currenttime;
  bool                     blockformatstarted;
  bool                     objparametersvalid;
};

BBC_AUDIOTOOLBOX_END

#endif
