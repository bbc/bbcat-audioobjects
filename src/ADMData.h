#ifndef __ADM_DATA__
#define __ADM_DATA__

#include <string>
#include <vector>
#include <map>

#include <aplibs-dsp/misc.h>

#include "ADMObjects.h"

BBC_AUDIOTOOLBOX_START

class ADMData {
public:
  ADMData();
  virtual ~ADMData();

  virtual void Delete();

  bool Set(const uint8_t *chna, const uint8_t *axml, uint_t axmllength);

  virtual bool SetChna(const uint8_t *data);
  virtual bool SetAxml(const uint8_t *data, uint_t length);
  bool SetAxml(const std::string& data);

  virtual void SortTracks();
  virtual void ConnectReferences();
  virtual void UpdateLimits();

  uint8_t    *GetChna(uint32_t& len) const;
  std::string GetAxml(const std::string& indent = "\t", const std::string& eol = "\n", uint_t ind_level = 0) const;

  void Register(ADMObject *obj, const std::string& type);

  typedef ADMObject::ADMVALUE ADMVALUE;
  ADMObject *GetReference(const ADMVALUE& value);

  void GetADMList(const std::string& type, std::vector<const ADMObject *>& list) const;
  const ADMObject *GetObjectByID(const std::string& id, const std::string& type = "") const;
  const ADMObject *GetObjectByName(const std::string& name, const std::string& type = "") const;

  typedef std::vector<const ADMAudioTrack *> TRACKLIST;
  const TRACKLIST& GetTrackList() const {return tracklist;}

  virtual void Dump(std::string& str, const std::string& indent = "  ", const std::string& eol = "\n", uint_t level = 0) const;
  virtual void GenerateXML(std::string& str, const std::string& indent = "\t", const std::string& eol = "\n", uint_t ind_level = 0) const;

  virtual void CreateCursors(std::vector<PositionCursor *>& list, uint_t channel = 0, uint_t nchannels = ~0) const;

  virtual void Serialize(uint8_t *dst, uint_t& len) const;

  static ADMData *Create();

  typedef ADMData *(*CREATOR)(void *context);
  static void RegisterProvider(CREATOR fn, void *context = NULL);

protected:
  typedef struct {
    std::string type;
    std::string id;
    std::string name;
  } ADMHEADER;

  virtual bool TranslateXML(const std::string& data) = 0;

  virtual bool ValidType(const std::string& type) const;

  virtual ADMObject *Create(const std::string& type, const std::string& id, const std::string& name);
  virtual ADMObject *Parse(const std::string& type, void *userdata);

  virtual void ParseHeader(ADMHEADER& header, const std::string& type, void *userdata) = 0;
  virtual void ParseValue(ADMObject *obj, const std::string& type, void *userdata) = 0;
  virtual void ParseValues(ADMObject *obj, const std::string& type, void *userdata) = 0;

  /*--------------------------------------------------------------------------------*/
  /** Optional post parse handler
   */
  /*--------------------------------------------------------------------------------*/
  virtual void PostParse(ADMObject *obj, const std::string& type, void *userdata) {
    UNUSED_PARAMETER(obj);
    UNUSED_PARAMETER(type);
    UNUSED_PARAMETER(userdata);
  }

  static std::string FormatString(const char *fmt, ...);

protected:
  typedef struct {
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
