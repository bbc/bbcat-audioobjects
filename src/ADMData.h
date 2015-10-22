#ifndef __ADM_DATA__
#define __ADM_DATA__

#include <string>
#include <vector>
#include <map>

#include <bbcat-base/misc.h>

#include "ADMObjects.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** ADM data class
 *
 * This class forms the base class of ADM reading and writing
 *
 * It CANNOT, by itself decode the XML from axml chunks, that must be done by a derived class.
 *
 * It CAN, however, generate XML (using a very simple method) from a manually created ADM
 *
 */
/*--------------------------------------------------------------------------------*/
class ADMData
{
public:
  ADMData();
  virtual ~ADMData();

  /*--------------------------------------------------------------------------------*/
  /** Delete all objects within this ADM
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Delete();

  /*--------------------------------------------------------------------------------*/
  /** Read ADM data from the chna and axml RIFF chunks
   *
   * @param chna ptr to chna chunk data 
   * @param chnalength length of chna data
   * @param axml ptr to axml chunk data (MUST be terminated like a string) 
   *
   * @return true if data read successfully
   *
   * @note this requires facilities from a derived class
   */
  /*--------------------------------------------------------------------------------*/
  bool Set(const uint8_t *chna, uint_t chnalength, const char *axml);

  /*--------------------------------------------------------------------------------*/
  /** Read ADM data from the chna RIFF chunk
   *
   * @param data ptr to chna chunk data 
   * @param len length of chna chunk
   *
   * @return true if data read successfully
   */
  /*--------------------------------------------------------------------------------*/
  bool SetChna(const uint8_t *data, uint_t len);

  /*--------------------------------------------------------------------------------*/
  /** Read ADM data from the axml RIFF chunk
   *
   * @param data ptr to axml chunk data (MUST be terminated!)
   *
   * @return true if data read successfully
   */
  /*--------------------------------------------------------------------------------*/
  bool SetAxml(const char *data);

  /*--------------------------------------------------------------------------------*/
  /** Read ADM data from explicit XML
   *
   * @param data XML data stored as a string
   *
   * @return true if data read successfully
   */
  /*--------------------------------------------------------------------------------*/
  bool SetAxml(const std::string& data);

  /*--------------------------------------------------------------------------------*/
  /** Load CHNA data from file
   *
   * @param filename file containing chna in text form
   *
   * @return true if data read successfully
   */
  /*--------------------------------------------------------------------------------*/
  bool ReadChnaFromFile(const std::string& filename);

  /*--------------------------------------------------------------------------------*/
  /** Load ADM data from file
   *
   * @param filename file containing XML
   *
   * @return true if data read successfully
   */
  /*--------------------------------------------------------------------------------*/
  bool ReadXMLFromFile(const std::string& filename);

  /*--------------------------------------------------------------------------------*/
  /** Set default 'pure' mode value
   */
  /*--------------------------------------------------------------------------------*/
  static void SetDefaultPureMode(bool enable = true) {defaultpuremode = enable;}

  /*--------------------------------------------------------------------------------*/
  /** Enable/disable 'pure' mode
   *
   * @param enable true to enable pure mode
   *
   * @note in pure mode, implied links and settings will *not* be applied
   * @note (e.g. setting type labels of referenced objects)
   */
  /*--------------------------------------------------------------------------------*/
  void EnablePureMode(bool enable = true) {puremode = enable;}
  bool InPureMode() const {return puremode;}

  /*--------------------------------------------------------------------------------*/
  /** Finalise ADM
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Finalise();

  /*--------------------------------------------------------------------------------*/
  /** Create chna chunk data
   *
   * @param len reference to length variable to be updated with the size of the chunk
   *
   * @return ptr to chunk data
   */
  /*--------------------------------------------------------------------------------*/
  uint8_t    *GetChna(uint32_t& len) const;

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
  std::string GetAxml(const std::string& indent = "\t", const std::string& eol = "\n", uint_t ind_level = 0) const;

  /*--------------------------------------------------------------------------------*/
  /** Create an ADM sub-object within this ADM object
   *
   * @param type object type - should always be the static 'Type' member of the object to be created (e.g. ADMAudioProgramme::Type)
   * @param id unique ID for the object (or empty string to have one created)
   * @param name human-readable name of the object
   *
   * @return ptr to object or NULL if type unrecognized or the object already exists
   */
  /*--------------------------------------------------------------------------------*/
  virtual ADMObject *Create(const std::string& type, const std::string& id, const std::string& name);

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
  std::string CreateID(const std::string& type);

  /*--------------------------------------------------------------------------------*/
  /** Change the ID of the specified object
   *
   * @param obj ADMObject to change ID of
   * @param id new ID
   * @param start starting index used for search
   */
  /*--------------------------------------------------------------------------------*/
  void ChangeID(ADMObject *obj, const std::string& id, uint_t start = 0);

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
  ADMAudioProgramme *CreateProgramme(const std::string& name);

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
  ADMAudioContent *CreateContent(const std::string& name, ADMAudioProgramme *programme = NULL);

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
  ADMAudioObject *CreateObject(const std::string& name, ADMAudioContent *content = NULL);

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
  ADMAudioPackFormat *CreatePackFormat(const std::string& name, ADMAudioObject *object = NULL);

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
  ADMAudioTrack *CreateTrack(const std::string& name, ADMAudioObject *object = NULL);

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
  ADMAudioChannelFormat *CreateChannelFormat(const std::string& name, ADMAudioPackFormat *packFormat = NULL, ADMAudioStreamFormat *streamFormat = NULL);

  /*--------------------------------------------------------------------------------*/
  /** Create audioBlockFormat object
   *
   * @param channelformat optional channel format to add the new block to
   *
   * @return ADMAudioBlockFormat object
   *
   * @note audioBlockFormat object MUST be added to an audioChannelFormat to be useful (but can be added after this call)!
   */
  /*--------------------------------------------------------------------------------*/
  ADMAudioBlockFormat *CreateBlockFormat(ADMAudioChannelFormat *channelformat);

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
  ADMAudioTrackFormat *CreateTrackFormat(const std::string& name, ADMAudioStreamFormat *streamFormat = NULL);

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
  ADMAudioStreamFormat *CreateStreamFormat(const std::string& name, ADMAudioTrackFormat *trackFormat = NULL);

  /*--------------------------------------------------------------------------------*/
  /** Register an ADM sub-object with this ADM
   *
   * @param obj ptr to ADM object
   *
   */
  /*--------------------------------------------------------------------------------*/
  void Register(ADMObject *obj);

  /*--------------------------------------------------------------------------------*/
  /** Return the object associated with the specified reference
   *
   * @param value a name/value pair specifying object type and name
   */
  /*--------------------------------------------------------------------------------*/
  ADMObject *GetReference(const XMLValue& value);

  /*--------------------------------------------------------------------------------*/
  /** Get list of objects of specified type
   *
   * @param type audioXXX object type (ADMAudioXXX::Type)
   * @param list list to be populated
   */
  /*--------------------------------------------------------------------------------*/
  void GetObjects(const std::string& type, std::vector<const ADMObject *>& list) const;

  /*--------------------------------------------------------------------------------*/
  /** Get list of writable objects of specified type
   *
   * @param type audioXXX object type (ADMAudioXXX::Type)
   * @param list list to be populated
   */
  /*--------------------------------------------------------------------------------*/
  void GetWritableObjects(const std::string& type, std::vector<ADMObject *>& list) const;

  /*--------------------------------------------------------------------------------*/
  /** Get ADM object by ID (with optional object type specified)
   *
   * @param id object ID
   * @param type audioXXX object type (ADMAudioXXX::Type)
   *
   * @return object or NULL
   */
  /*--------------------------------------------------------------------------------*/
  const ADMObject *GetObjectByID(const std::string& id, const std::string& type = "") const;

  /*--------------------------------------------------------------------------------*/
  /** Get ADM object by Name (with optional object type specified)
   *
   * @param name object name
   * @param type audioXXX object type (ADMAudioXXX::Type)
   *
   * @return object or NULL
   */
  /*--------------------------------------------------------------------------------*/
  const ADMObject *GetObjectByName(const std::string& name, const std::string& type = "") const;

  /*--------------------------------------------------------------------------------*/
  /** Get writable ADM object by ID (with optional object type specified)
   *
   * @param id object ID
   * @param type audioXXX object type (ADMAudioXXX::Type)
   *
   * @return object or NULL
   */
  /*--------------------------------------------------------------------------------*/
  ADMObject *GetWritableObjectByID(const std::string& id, const std::string& type = "");

  /*--------------------------------------------------------------------------------*/
  /** Get writable ADM object by Name (with optional object type specified)
   *
   * @param name object name
   * @param type audioXXX object type (ADMAudioXXX::Type)
   *
   * @return object or NULL
   */
  /*--------------------------------------------------------------------------------*/
  ADMObject *GetWritableObjectByName(const std::string& name, const std::string& type = "");

  /*--------------------------------------------------------------------------------*/
  /** Return a list of all ADM Audio Objects
   */
  /*--------------------------------------------------------------------------------*/
  void GetAudioObjectList(std::vector<const ADMAudioObject *>& list) const;

  /*--------------------------------------------------------------------------------*/
  /** Return track list (list of ADMAudioTrack objects)
   */
  /*--------------------------------------------------------------------------------*/
  typedef std::vector<const ADMAudioTrack *> TRACKLIST;
  const TRACKLIST& GetTrackList() const {return tracklist;}

  /*--------------------------------------------------------------------------------*/
  /** Structure containing names of ADM objects to create and connect together using CreateObjects()
   *
   * If the required object of the specified name does not exist, it is created
   *
   * Objects are connected according to the ADM rules, as many objects in the structure
   * are connected together as possible
   *
   * Any empty names will be ignored and objects not created or connected
   */
  /*--------------------------------------------------------------------------------*/
  typedef struct
  {
    uint_t      trackNumber;            ///< physical track number to use (1- based) (NOTE: tracks are never created by CreateObjects())
    std::string programmeName;          ///< programme title
    std::string contentName;            ///< content title
    std::string objectName;             ///< object name
    std::string packFormatName;         ///< pack format name
    std::string channelFormatName;      ///< channel format name
    std::string streamFormatName;       ///< stream format name
    std::string trackFormatName;        ///< track format name
    uint_t      typeLabel;              ///< default typeLabel for packs, channels, streams and tracks
    struct {
      ADMAudioProgramme     *programme;
      ADMAudioContent       *content;
      ADMAudioObject        *object;
      ADMAudioPackFormat    *packFormat;
      ADMAudioChannelFormat *channelFormat;
      ADMAudioStreamFormat  *streamFormat;
      ADMAudioTrackFormat   *trackFormat;
      ADMAudioTrack         *audioTrack;
    } objects;
  } OBJECTNAMES;

  /*--------------------------------------------------------------------------------*/
  /** Create/link ADM objects
   *
   * @param data OBJECTNAMES structure populated with names of objects to link/create (see above)
   *
   * @return true if successful
   */
  /*--------------------------------------------------------------------------------*/
  bool CreateObjects(OBJECTNAMES& names);

  /*--------------------------------------------------------------------------------*/
  /** Generate a mapping from objects of type <type1> to/from objects of type <type2>
   *
   * @param refmap map to be populated
   * @param type1 first type (ADMAudioXXX:TYPE>
   * @param type2 second type (ADMAudioXXX:TYPE>
   * @param reversed false for type1 -> type2, true for type2 -> type1
   */
  /*--------------------------------------------------------------------------------*/
  typedef std::map<const ADMObject *,std::vector<const ADMObject *> > ADMREFERENCEMAP;
  void GenerateReferenceMap(ADMREFERENCEMAP& refmap, const std::string& type1, const std::string& type2, bool reversed = false) const;

  /*--------------------------------------------------------------------------------*/
  /** Update audio object limits
   */
  /*--------------------------------------------------------------------------------*/
  void UpdateAudioObjectLimits();

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
  virtual void Dump(std::string& str, const ADMObject *obj = NULL, const std::string& indent = "  ", const std::string& eol = "\n", uint_t level = 0) const;

  /*--------------------------------------------------------------------------------*/
  /** Create XML representation of ADM
   *
   * @param str std::string to be modified with XML
   * @param indent indentation for each level of objects
   * @param eol end-of-line string
   * @param level initial indentation level
   *
   * @return total length of XML
   *
   * @note for other XML implementaions, this function can be overridden
   */
  /*--------------------------------------------------------------------------------*/
  virtual uint64_t GenerateXML(std::string& str, const std::string& indent = "\t", const std::string& eol = "\n", uint_t ind_level = 0, bool complete = false) const;

  /*--------------------------------------------------------------------------------*/
  /** Create XML representation of ADM
   *
   * @param buf buffer to store XML in (or NULL to just get the length)
   * @param buflen maximum length of buf (excluding terminator)
   * @param indent indentation for each level of objects
   * @param eol end-of-line string
   * @param level initial indentation level
   *
   * @return total length of XML
   *
   * @note for other XML implementaions, this function can be overridden
   */
  /*--------------------------------------------------------------------------------*/
  virtual uint64_t GenerateXMLBuffer(uint8_t *buf, uint64_t buflen, const std::string& indent = "\t", const std::string& eol = "\n", uint_t ind_level = 0, bool complete = false) const;

  /*--------------------------------------------------------------------------------*/
  /** Generate a textual list of references 
   *
   * @param str string to be modified
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GenerateReferenceList(std::string& str) const;

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
  bool CreateFromFile(const char *filename);

  static const std::string DefaultStandardDefinitionsFile;
  static ADMData *Create(const std::string& standarddefinitionsfile = "");

  typedef ADMData *(*CREATOR)(const std::string& standarddefinitionsfile, void *context);
  static void RegisterProvider(CREATOR fn, void *context = NULL);

protected:
  /*--------------------------------------------------------------------------------*/
  /** Load standard definitions file into ADM
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool LoadStandardDefinitions(const std::string& filename);

  typedef struct
  {
    std::string type;
    std::string id;
    std::string name;
  } ADMHEADER;

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
  std::string FindUniqueID(const std::string& type, const std::string& format, uint_t start);

  /*--------------------------------------------------------------------------------*/
  /** Decode XML string as ADM
   *
   * @param data ptr to string containing ADM XML (MUST be terminated)
   *
   * @return true if XML decoded correctly
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool TranslateXML(const char *data) = 0;

  virtual bool ValidType(const std::string& type) const;

  virtual ADMObject *Parse(const std::string& type, void *userdata);

  /*--------------------------------------------------------------------------------*/
  /** Parse XML header
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ParseHeader(ADMHEADER& header, const std::string& type, void *userdata) = 0;

  /*--------------------------------------------------------------------------------*/
  /** Parse attributes and subnodes as values
   *
   * @param obj object to read values from
   * @param userdata user suppled data
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ParseValues(ADMObject *obj, void *userdata) = 0;

  /*--------------------------------------------------------------------------------*/
  /** Optional post parse handler
   */
  /*--------------------------------------------------------------------------------*/
  virtual void PostParse(ADMObject *obj, void *userdata)
  {
    UNUSED_PARAMETER(obj);
    UNUSED_PARAMETER(userdata);
  }

  /*--------------------------------------------------------------------------------*/
  /** From a list of objects, find all objects that are referenced
   *
   * @param list initial list of objects - will be EXPANDED with more objects
   *
   */
  /*--------------------------------------------------------------------------------*/
  void GetReferencedObjects(std::vector<const ADMObject *>& list) const;

  /*--------------------------------------------------------------------------------*/
  /** Generic XML creation
   *
   * @param xmlcontext user supplied argument representing context data
   *
   * @note for other XML implementaions, this function can be overridden
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GenerateXML(void *xmlcontext) const;

  /*--------------------------------------------------------------------------------*/
  /** Generic XML creation
   *
   * @param obj ADM object to generate XML for
   * @param xmlcontext user supplied argument representing context data
   *
   * @note for other XML implementaions, this function can be overridden
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GenerateXML(const ADMObject *obj, void *xmlcontext) const;

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
  virtual void StartXML(void *xmlcontext, const std::string& version = "1.0", const std::string& encoding = "UTF-8") const;
  /*--------------------------------------------------------------------------------*/
  /** Start an XML object
   *
   * @param xmlcontext user supplied argument representing context data
   * @param name object name
   *
   * @note for other XML implementaions, this function MUST be overridden
   */
  /*--------------------------------------------------------------------------------*/
  virtual void OpenXMLObject(void *xmlcontext, const std::string& name) const;
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
  virtual void AddXMLAttribute(void *xmlcontext, const std::string& name, const std::string& value) const;
  /*--------------------------------------------------------------------------------*/
  /** Add attributes from list of XML values to current object header
   *
   * @param xmlcontext user supplied argument representing context data
   * @param values list of XML values
   *
   * @note for other XML implementaions, this function MUST be overridden
   */
  /*--------------------------------------------------------------------------------*/
  virtual void AddXMLAttributes(void *xmlcontext, const XMLValues& values) const;
  /*--------------------------------------------------------------------------------*/
  /** Add values (non-attributres) from list of XML values to object
   *
   * @param xmlcontext user supplied argument representing context data
   * @param values list of XML values
   *
   * @note for other XML implementaions, this function MUST be overridden
   */
  /*--------------------------------------------------------------------------------*/
  virtual void AddXMLValues(void *xmlcontext, const XMLValues& values) const;
  /*--------------------------------------------------------------------------------*/
  /** Set XML data
   *
   * @param xmlcontext user supplied argument representing context data
   * @param data data
   *
   * @note for other XML implementaions, this function MUST be overridden
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetXMLData(void *xmlcontext, const std::string& data) const;
  /*--------------------------------------------------------------------------------*/
  /** Close XML object
   *
   * @param xmlcontext user supplied argument representing context data
   *
   * @note for other XML implementaions, this function MUST be overridden
   */
  /*--------------------------------------------------------------------------------*/
  virtual void CloseXMLObject(void *xmlcontext) const;

  /*--------------------------------------------------------------------------------*/
  /** Append string to XML context
   *
   * @param xmlcontext user supplied argument representing context data
   * @param str string to be appended/counted
   */
  /*--------------------------------------------------------------------------------*/
  virtual void AppendXML(void *xmlcontext, const std::string& str) const;

  /*--------------------------------------------------------------------------------*/
  /** Context structure for generating XML (this object ONLY!)
   */
  /*--------------------------------------------------------------------------------*/
  typedef struct {
    struct {
      std::string *str;                 ///< ptr string into which XML is generated (or NULL)
      char        *buf;                 ///< raw char buffer (PRE-ALLOCATED) to generate XML in
      uint64_t    buflen;               ///< max length of above length (EXCLUDING terminator)
    } destination;                      ///< destination control
    std::string indent;                 ///< indent string for each level
    std::string eol;                    ///< end-of-line string
    uint64_t    length;                 ///< length of XML data at completion
    uint_t      ind_level;              ///< current indentation level
    bool        opened;                 ///< true if object is started but not ready for data (needs '>')
    bool        complete;               ///< dump ALL objects, not just programme, content or objects
    bool        eollast;                ///< string currently ends with an eol
    std::vector<std::string> stack;     ///< object stack
  } TEXTXML;

  /*--------------------------------------------------------------------------------*/
  /** Context structure for generating textual dump of ADM
   */
  /*--------------------------------------------------------------------------------*/
  typedef struct {
    std::string str;                    ///< string into which text is generated
    std::string indent;                 ///< indent string for each level
    std::string eol;                    ///< end-of-line string
    uint_t      ind_level;              ///< current indentation level
  } DUMPCONTEXT;
  /*--------------------------------------------------------------------------------*/
  /** Generate textual description of ADM object (recursive
   *
   * @param obj ADM object
   * @param map map of objects already stored (bool is a dummy)
   * @param context DUMPCONTEXT for tracking indentation, etc
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Dump(const ADMObject *obj, std::map<const ADMObject *,bool>& map, DUMPCONTEXT& context) const;

  /*--------------------------------------------------------------------------------*/
  /** Connect XML references once all objects have been read
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ConnectReferences();

  /*--------------------------------------------------------------------------------*/
  /** Sort tracks into numerical order
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SortTracks();

  /*--------------------------------------------------------------------------------*/
  /** Change temporary ID of object and all its referenced objects
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ChangeTemporaryID(ADMObject *obj, std::map<ADMObject *,bool>& map);

  /*--------------------------------------------------------------------------------*/
  /** Change temporary IDs to full valid ones based on a set of rules
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ChangeTemporaryIDs();

#if ENABLE_JSON
  /*--------------------------------------------------------------------------------*/
  /** Return ADM as JSON
   */
  /*--------------------------------------------------------------------------------*/
  virtual json_spirit::mObject ToJSON() const;
#endif

protected:
  typedef struct
  {
    CREATOR fn;
    void    *context;
  } PROVIDER;

  /*--------------------------------------------------------------------------------*/
  /** Return provider list, creating as necessary
   */
  /*--------------------------------------------------------------------------------*/
  static std::vector<PROVIDER>& GetProviderList();

  typedef std::map<std::string,ADMObject*> ADMOBJECTS_MAP;
  typedef ADMOBJECTS_MAP::iterator         ADMOBJECTS_IT;
  typedef ADMOBJECTS_MAP::const_iterator   ADMOBJECTS_CIT;

protected:
  ADMOBJECTS_MAP               admobjects;
  TRACKLIST                    tracklist;
  std::map<std::string,uint_t> uniqueids;
  bool                         puremode;

  static const std::string     tempidsuffix;
  static bool                  defaultpuremode;
};

BBC_AUDIOTOOLBOX_END

#endif
