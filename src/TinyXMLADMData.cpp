
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <tinyxml.h>

#define DEBUG_LEVEL 1
#include "TinyXMLADMData.h"

BBC_AUDIOTOOLBOX_START

TinyXMLADMData::TinyXMLADMData()
{
  // no special construction required
}

TinyXMLADMData::~TinyXMLADMData()
{
  // no special destruction required
}

/*--------------------------------------------------------------------------------*/
/** Required implementation of XML translation
 */
/*--------------------------------------------------------------------------------*/
bool TinyXMLADMData::TranslateXML(const std::string& data)
{
  TiXmlDocument doc;
  const TiXmlNode *node;
  bool success = false;

  DEBUG3(("XML: %s", data.c_str()));

  // dig to correct location of audioFormatExtended section
  doc.Parse(data.c_str());
  if ((node = FindElement(&doc, "ebuCoreMain")) != NULL)
  {
    if ((node = FindElement(node, "coreMetadata")) != NULL)
    {
      if ((node = FindElement(node, "format")) != NULL)
      {
        if ((node = FindElement(node, "audioFormatExtended")) != NULL)
        {
          CollectObjects(node);
                    
          success = true;
        }
        else ERROR("Failed to find audioFormatExtended element");
      }
      else ERROR("Failed to find format element");
    }
    else ERROR("Failed to find coreMetadata element");
  }
  else ERROR("Failed to find ebuCoreMain element");

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Parse the XML section header
 *
 * @param header header object ot be populated
 * @param type XML type of object
 * @param userdata user suppled data
 *
 */
/*--------------------------------------------------------------------------------*/
void TinyXMLADMData::ParseHeader(ADMHEADER& header, const std::string& type, void *userdata)
{
  const TiXmlNode      *node = (const TiXmlNode *)userdata;
  const TiXmlAttribute *attr; 
  char dummyid[32];

  // set up dummy, unique ID
  snprintf(dummyid, sizeof(dummyid) - 1, "%08lx", (ulong_t)GetTickCount());

  header.type = type;
  header.id   = dummyid;

  for (attr = node->ToElement()->FirstAttribute(); attr; attr = attr->Next())
  {
    std::string attr_name = attr->Name();

    // find '<type>Name', '<type>ID' or 'UID' (special for AudioTrack)
    if (attr_name == (type + "Name"))
    {
      header.name = attr->Value();
    }
    else if (attr_name == (type + "ID"))
    {
      header.id   = attr->Value();
    }
    else if (attr_name == "UID")
    {
      header.id   = attr->Value();
    }
  }

  DEBUG2(("Parse header (type='%s', id='%s', name='%s')", header.type.c_str(), header.id.c_str(), header.name.c_str()));
}

void TinyXMLADMData::ParseValue(ADMObject *obj, const std::string& type, void *userdata)
{
  const TiXmlNode      *node    = (const TiXmlNode *)userdata;
  const TiXmlNode      *subnode = node->FirstChild();
  const TiXmlAttribute *attr; 
  ADMVALUE value;

  UNUSED_PARAMETER(type);

  value.attr = false;
  value.name = node->Value();
  if (subnode) value.value = subnode->Value();

  DEBUG3(("%s: %s='%s', attrs:",
          obj->ToString().c_str(),
          value.name.c_str(), value.value.c_str()));
    
  for (attr = node->ToElement()->FirstAttribute(); attr; attr = attr->Next())
  {
    value.attrs[attr->Name()] = attr->Value();

    DEBUG3(("\t%s='%s'", attr->Name(), attr->Value()));
  }

  obj->AddValue(value);
}

/*--------------------------------------------------------------------------------*/
/** Parse attributes and subnodes as values
 *
 * @param obj object to read values from
 * @param type XML type of object
 * @param userdata user suppled data
 *
 */
/*--------------------------------------------------------------------------------*/
void TinyXMLADMData::ParseValues(ADMObject *obj, const std::string& type, void *userdata)
{
  // if the supplied ADM object is an AudioChannelFormat, AudioBlockFormat sub-sections will be parsed
  ADMAudioChannelFormat *channel = dynamic_cast<ADMAudioChannelFormat *>(obj);
  const TiXmlNode       *node    = (const TiXmlNode *)userdata;
  const TiXmlAttribute  *attr; 
  const TiXmlNode       *subnode; 

  // parse attributes
  for (attr = node->ToElement()->FirstAttribute(); attr; attr = attr->Next())
  {
    std::string attr_name = attr->Name();

    // ignore header values previously found
    if ((attr_name != (type + "Name")) &&
        (attr_name != (type + "ID")) &&
        (attr_name != "UID"))
    {
      ADMVALUE value;
            
      value.attr  = true;
      value.name  = attr_name;
      value.value = attr->Value();

      obj->AddValue(value);

      DEBUG3(("%s: %s='%s'",
              obj->ToString().c_str(),
              attr_name.c_str(), attr->Value()));
    }
  }

  // parse subnode elements and create values from them
  for (subnode = node->FirstChild(); subnode; subnode = subnode->NextSibling())
  {
    if (subnode->Type() == TiXmlNode::TINYXML_ELEMENT)
    {
      std::string name = subnode->Value();

      if (name == ADMAudioBlockFormat::Type)
      {
        // AudioBlockFormat sections are handled differently...

        if (channel)
        {
          // if the supplied ADM object is an AudioChannelFormat, parse this subnode as an AudioBlockFormat section
          ADMAudioBlockFormat *block;

          // parse this subnode as a section
          if ((block = dynamic_cast<ADMAudioBlockFormat *>(Parse(name, (void *)subnode))) != NULL)
          {
            channel->Add(block);
          }
          else ERROR("Parsed object was not an AudioBlockFormat object");
        }
        else ERROR("No AudioChannelFormat for found AudioBlockFormat");
      }
      else ParseValue(obj, type, (void *)subnode);
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Find named element in subnode list
 *
 * @param node parent node
 * @param name name of subnode to find
 */
/*--------------------------------------------------------------------------------*/
const TiXmlNode *TinyXMLADMData::FindElement(const TiXmlNode *node, const std::string& name)
{
  const TiXmlNode *subnode; 

  // iterate through subnodes to find named element
  for (subnode = node->FirstChild(); subnode; subnode = subnode->NextSibling())
  {
    if ((subnode->Type() == TiXmlNode::TINYXML_ELEMENT) &&
        (name == subnode->Value()))
    {
      return subnode;
    }
  }

  return NULL;
}

/*--------------------------------------------------------------------------------*/
/** Recursively collect all objects in tree and parse them
 */
/*--------------------------------------------------------------------------------*/
void TinyXMLADMData::CollectObjects(const TiXmlNode *node)
{
  const TiXmlNode *subnode; 

  for (subnode = node->FirstChild(); subnode; subnode = subnode->NextSibling())
  {
    if (subnode->Type() == TiXmlNode::TINYXML_ELEMENT)
    {
      if (ValidType(subnode->Value()))
      {
        if (Parse(subnode->Value(), (void *)subnode))
        {
          CollectObjects(subnode);
        }
      }
    }
  }
}

BBC_AUDIOTOOLBOX_END
