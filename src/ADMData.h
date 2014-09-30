#ifndef __ADM_DATA__
#define __ADM_DATA__

#include <string>
#include <vector>
#include <map>

#include <aplibs-dsp/misc.h>

#include "ADMObjects.h"

BBC_AUDIOTOOLBOX_START

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
   * @param axml ptr to axml chunk data 
   * @param axmllength length of axml data
   *
   * @return true if data read successfully
   */
  /*--------------------------------------------------------------------------------*/
  bool Set(const uint8_t *chna, const uint8_t *axml, uint_t axmllength);

  /*--------------------------------------------------------------------------------*/
  /** Read ADM data from the chna RIFF chunk
   *
   * @param data ptr to chna chunk data 
   *
   * @return true if data read successfully
   */
  /*--------------------------------------------------------------------------------*/
  bool SetChna(const uint8_t *data);

  /*--------------------------------------------------------------------------------*/
  /** Read ADM data from the axml RIFF chunk
   *
   * @param data ptr to axml chunk data 
   * @param length length of axml data
   *
   * @return true if data read successfully
   */
  /*--------------------------------------------------------------------------------*/
  bool SetAxml(const uint8_t *data, uint_t length);

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
  /** Connect XML references once all objects have been read
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ConnectReferences();

  /*--------------------------------------------------------------------------------*/
  /** Update audio start and stop times once all objects have been read
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdateLimits();

  /*--------------------------------------------------------------------------------*/
  /** Sort tracks into numerical order
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SortTracks();

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
   * @param id unique ID for the object (or empty string to create one using CreateID())
   * @param name human-readable name of the object
   *
   * @return ptr to object or NULL if type unrecognized or the object already exists
   */
  /*--------------------------------------------------------------------------------*/
  virtual ADMObject *Create(const std::string& type, const std::string& id, const std::string& name, const ADMAudioChannelFormat *channelformat = NULL);

  /*--------------------------------------------------------------------------------*/
  /** Create an unique ID for the specified type
   *
   * @param type object type - should always be the static 'Type' member of the object to be created (e.g. ADMAudioProgramme::Type)
   * @param channelformat ptr to channelformat object if type is ADMAudioBlockFormat::Type
   *
   * @return unique ID
   */
  /*--------------------------------------------------------------------------------*/
  std::string CreateID(const std::string& type, const ADMAudioChannelFormat *channelformat = NULL) const;

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
   * @param name name of object
   * @param channelFormat audioChannelFormat object to attach this object to or NULL
   *
   * @note ID will be create automatically
   *
   * @return ADMAudioBlockFormat object
   */
  /*--------------------------------------------------------------------------------*/
  ADMAudioBlockFormat *CreateBlockFormat(const std::string& name, ADMAudioChannelFormat *channelFormat = NULL);

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
  typedef ADMObject::ADMVALUE ADMVALUE;
  ADMObject *GetReference(const ADMVALUE& value);

  void GetADMList(const std::string& type, std::vector<const ADMObject *>& list) const;
  const ADMObject *GetObjectByID(const std::string& id, const std::string& type = "") const;
  const ADMObject *GetObjectByName(const std::string& name, const std::string& type = "") const;

  typedef std::vector<const ADMAudioTrack *> TRACKLIST;
  const TRACKLIST& GetTrackList() const {return tracklist;}

  virtual void Dump(std::string& str, const std::string& indent = "  ", const std::string& eol = "\n", uint_t level = 0) const;
  virtual void GenerateXML(std::string& str, const std::string& indent = "\t", const std::string& eol = "\n", uint_t ind_level = 0) const;

  virtual void GenerateReferenceList(std::string& str);

  virtual void CreateCursors(std::vector<PositionCursor *>& list, uint_t channel = 0, uint_t nchannels = ~0) const;

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

  static ADMData *Create();

  typedef ADMData *(*CREATOR)(void *context);
  static void RegisterProvider(CREATOR fn, void *context = NULL);

protected:
  typedef struct
  {
    std::string type;
    std::string id;
    std::string name;
  } ADMHEADER;

  virtual bool TranslateXML(const std::string& data) = 0;

  virtual bool ValidType(const std::string& type) const;

  virtual ADMObject *Parse(const std::string& type, void *userdata);

  virtual void ParseHeader(ADMHEADER& header, const std::string& type, void *userdata) = 0;
  virtual void ParseValue(ADMObject *obj, const std::string& type, void *userdata) = 0;
  virtual void ParseValues(ADMObject *obj, const std::string& type, void *userdata) = 0;

  /*--------------------------------------------------------------------------------*/
  /** Optional post parse handler
   */
  /*--------------------------------------------------------------------------------*/
  virtual void PostParse(ADMObject *obj, const std::string& type, void *userdata)
  {
    UNUSED_PARAMETER(obj);
    UNUSED_PARAMETER(type);
    UNUSED_PARAMETER(userdata);
  }

  static std::string FormatString(const char *fmt, ...);

protected:
  typedef struct
  {
    CREATOR fn;
    void    *context;
  } PROVIDER;

  typedef std::map<std::string,ADMObject*> ADMOBJECTS_MAP;
  typedef ADMOBJECTS_MAP::iterator         ADMOBJECTS_IT;
  typedef ADMOBJECTS_MAP::const_iterator   ADMOBJECTS_CIT;

protected:
  ADMOBJECTS_MAP admobjects;
  TRACKLIST      tracklist;

  static std::vector<PROVIDER> providerlist;
};

BBC_AUDIOTOOLBOX_END

#endif
