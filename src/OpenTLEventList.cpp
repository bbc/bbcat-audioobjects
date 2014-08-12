
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DEBUG_LEVEL 1
#include "OpenTLEventList.h"
#include "ADMData.h"

BBC_AUDIOTOOLBOX_START

OpenTLEventList::OpenTLEventList()
{
}

OpenTLEventList::~OpenTLEventList()
{
}

std::string OpenTLEventList::RemoveSpeakerSuffix(const std::string &str) const
{
  static const std::string suffices[] = {
    " R",
    " C",
    " L",
    " Rs",
    " Ls",
    " LFE",
  };
  std::string res = str;
  uint_t i;

  for (i = 0; i < NUMBEROF(suffices); i++)
  {
    size_t pos;
    
    if ((res.size() >= suffices[i].size()) &&
        (res.substr(pos = (res.size() - suffices[i].size())) == suffices[i]))
    {
      res = res.substr(0, pos);
      break;
    }
  }

  return res;
}

bool OpenTLEventList::Readfile(const char *filename)
{
  FILE *fp;
  bool success = false;

  if ((fp = fopen(filename, "rb")) != NULL)
  {
    fseek(fp, 0, SEEK_END);
    
    long len = ftell(fp);

    rewind(fp);

    char *str;
    if ((str = new char[len + 1]) != NULL)
    {
      if ((len = fread(str, sizeof(*str), len, fp)) > 0)
      {
        const char *p, *p1;

        str[len] = 0;

        DEBUG3(("OpenTL segment is %ld chars long", len));

        success = true;

        // extract track name
        if (((p = strstr(str, "TKNM(\"")) != NULL) && ((p1 = strstr(p + 6, "\")")) != NULL))
        {
          name.assign(p + 6, p1 - p - 6);
          objectname = RemoveSpeakerSuffix(name);
        }
        else
        {
          ERROR("Unable to extract track name from '%s'", str);
          success = false;
        }

        p = str;
        while (success && ((p = strstr(p, "EVNT{")) != NULL))
        {
          p += 5;
          
          const char *namep  = strstr(p, "EDNM(\"");
          const char *startp = strstr(p, "EDPT(");
          const char *lenp   = strstr(p, "EDLN(");
          
          if (namep && startp && lenp)
          {
            const char *namep1;
            EVENT ev;

            namep  += 6;
            startp += 5;
            lenp   += 5;

            if ((namep1 = strstr(namep, "\")")) != NULL)
            {
              ev.name.assign(namep, namep1 - namep);
            }
            else
            {
              ERROR("Failed to find name end marker from '%s'", namep);
              success = false;
              break;
            }

            if (sscanf(startp, "%lu", &ev.start) < 1)
            {
              ERROR("Failed to decode start time from '%s'", startp);
              success = false;
              break;
            }

            if (sscanf(lenp, "%lu", &ev.length) < 1)
            {
              ERROR("Failed to decode length from '%s'", lenp);
              success = false;
              break;
            }

            ev.objectname = RemoveSpeakerSuffix(ev.name.substr(0, ev.name.find("_")));
            if (objectcount.find(ev.objectname) == objectcount.end()) objectcount[ev.objectname] = 1;
            else                                                      objectcount[ev.objectname]++;

            Printf(ev.objectname, "_%u", objectcount[ev.objectname]);

            list.push_back(ev);
          }
          else ERROR("Failed to find name start marker, start marker or length marker from '%s'", p);
        }
      }
      else ERROR("Failed to read data (%ld), error %s", len, strerror(errno));

      delete[] str;
    }
  }

  return success;
}

void OpenTLEventList::Dump()
{
  uint_t i;

  for (i = 0; i < list.size(); i++)
  {
    const EVENT ev = list[i];
 
    DEBUG1(("Event %u/%u: name '%s' start %lu length %lu", i + 1, (uint_t)list.size(), ev.name.c_str(), ev.start, ev.length));
  }
}

BBC_AUDIOTOOLBOX_END
