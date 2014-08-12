#ifndef __DECODE_OPEN_TL__
#define __DECODE_OPEN_TL__

#include <string>
#include <vector>
#include <map>

#include <aplibs-dsp/misc.h>

BBC_AUDIOTOOLBOX_START

class OpenTLEventList
{
public:
  OpenTLEventList();
  ~OpenTLEventList();

  bool Readfile(const char *filename);

  const std::string& GetName()       const {return name;}
  const std::string& GetObjectName() const {return objectname;}

  typedef struct {
    std::string name;
    std::string objectname;
    ulong_t     start;
    ulong_t     length;
  } EVENT;
  typedef std::vector<EVENT> EVENTLIST;
  const EVENTLIST& GetEventList() const {return list;}

  void Dump();

protected:
  std::string RemoveSpeakerSuffix(const std::string &str) const;

protected:
  std::string                  name;
  std::string                  objectname;
  std::vector<EVENT>           list;
  std::map<std::string,uint_t> objectcount;
};

BBC_AUDIOTOOLBOX_END

#endif
