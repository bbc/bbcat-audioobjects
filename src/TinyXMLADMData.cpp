
#include <math.h>

#include <tinyxml.h>

#define DEBUG_LEVEL 1
#include "TinyXMLADMData.h"

BBC_AUDIOTOOLBOX_START

BBC_AUDIOTOOLBOX_KEEP(TinyXMLADMData);

const bool TinyXMLADMData::registered = TinyXMLADMData::Register();

TinyXMLADMData::TinyXMLADMData(const std::string& standarddefinitionsfile) : ADMData()
{
  LoadStandardDefinitions(standarddefinitionsfile);
}

TinyXMLADMData::~TinyXMLADMData()
{
  // no special destruction required
}

/*--------------------------------------------------------------------------------*/
/** Register function - this is called automatically
 */
/*--------------------------------------------------------------------------------*/
bool TinyXMLADMData::Register()
{
  static bool registered = false;
  if (!registered) {
    RegisterProvider(&__Creator);
    registered = true;
  }

  return registered;
}

/*--------------------------------------------------------------------------------*/
/** Decode XML string as ADM
 *
 * @param data ptr to string containing ADM XML (MUST be terminated)
 *
 * @return true if XML decoded correctly
 */
/*--------------------------------------------------------------------------------*/
bool TinyXMLADMData::TranslateXML(const char *data)
{
  TiXmlDocument doc;
  const TiXmlNode *node;
  bool success = false;

  DEBUG3(("XML: %s", data));

  doc.Parse(data);

  // dig to correct location of audioFormatExtended section
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
  else if ((node = FindElement(&doc, "ituADM")) != NULL)
  {
    const TiXmlNode *formatNode;
    if ((formatNode = FindElement(node, "audioFormatExtended")) != NULL)
    {
      CollectObjects(formatNode);
                    
      success = true;
    }
    else if ((node = FindElement(node, "coreMetadata")) != NULL)
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
    else ERROR("Failed to find audioFormatExtended element");
  }
  else ERROR("Failed to find ebuCoreMain or ituADM elements");

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

  header.type = type;
  header.id   = "";
  Printf(header.id, "%08lx", (ulong_t)GetNanosecondTicks());
  
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

/*--------------------------------------------------------------------------------*/
/** Parse value (and its attributes) into a list of XML values
 *
 * @param obj ADM object
 * @param userdata implementation specific object data
 */
/*--------------------------------------------------------------------------------*/
void TinyXMLADMData::ParseValue(ADMObject *obj, void *userdata)
{
  ParseValue(obj->ToString(), obj->GetValues(), userdata);
}

/*--------------------------------------------------------------------------------*/
/** Parse value (and its attributes) into a list of XML values
 *
 * @param name name of object (for DEBUGGING only)
 * @param values list of XML values to be added to
 * @param userdata implementation specific object data
 */
/*--------------------------------------------------------------------------------*/
void TinyXMLADMData::ParseValue(const std::string& name, XMLValues& values, void *userdata)
{
  const TiXmlNode      *node    = (const TiXmlNode *)userdata;
  const TiXmlNode      *subnode = node->FirstChild();
  const TiXmlAttribute *attr; 
  XMLValue value;

  UNUSED_PARAMETER(name);

  value.attr = false;
  value.name = node->Value();
  if (subnode) value.value = subnode->Value();

  DEBUG3(("%s: %s='%s', attrs:",
          name.c_str(),
          value.name.c_str(), value.value.c_str()));
    
  for (attr = node->ToElement()->FirstAttribute(); attr; attr = attr->Next())
  {
    value.attrs[attr->Name()] = attr->Value();

    DEBUG3(("\t%s='%s'", attr->Name(), attr->Value()));
  }

  values.AddValue(value);
}

/*--------------------------------------------------------------------------------*/
/** Parse attributes into a list of XML values
 *
 * @param obj ADM object
 * @param userdata implementation specific object data
 */
/*--------------------------------------------------------------------------------*/
void TinyXMLADMData::ParseAttributes(ADMObject *obj, void *userdata)
{
  ParseAttributes(obj->ToString(), obj->GetType(), obj->GetValues(), userdata);
}

/*--------------------------------------------------------------------------------*/
/** Parse attributes into a list of XML values
 *
 * @param name name of object (for DEBUGGING only)
 * @param type object type - necessary to prevent object name and ID being added
 * @param values list of XML values to be populated
 * @param userdata implementation specific object data
 */
/*--------------------------------------------------------------------------------*/
void TinyXMLADMData::ParseAttributes(const std::string& name, const std::string& type, XMLValues& values, void *userdata)
{
  const TiXmlNode      *node = (const TiXmlNode *)userdata;
  const TiXmlAttribute *attr; 

  UNUSED_PARAMETER(name);
  
  for (attr = node->ToElement()->FirstAttribute(); attr; attr = attr->Next())
  {
    std::string attr_name = attr->Name();

    // ignore header values previously found
    if ((attr_name != (type + "Name")) &&
        (attr_name != (type + "ID")) &&
        (attr_name != "UID"))
    {
      XMLValue value;
            
      value.attr  = true;
      value.name  = attr_name;
      value.value = attr->Value();

      values.AddValue(value);

      DEBUG3(("%s: %s='%s'",
              name.c_str(),
              attr_name.c_str(), attr->Value()));
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Parse audioBlockFormat XML object
 *
 * @param name parent name (for DEBUGGING only)
 * @param obj audioBlockFormat object
 * @param userdata user suppled data
 */
/*--------------------------------------------------------------------------------*/
void TinyXMLADMData::ParseValues(const std::string& name, ADMAudioBlockFormat *obj, void *userdata)
{
  const TiXmlNode *node = (const TiXmlNode *)userdata;
  const TiXmlNode *subnode;
  XMLValues       values;
            
  // parse attributes
  ParseAttributes(name, obj->GetType(), values, userdata);

  // parse subnode elements and create values from them
  for (subnode = node->FirstChild(); subnode; subnode = subnode->NextSibling())
  {
    if (subnode->Type() == TiXmlNode::TINYXML_ELEMENT)
    {
      ParseValue(name, values, (void *)subnode);
    }
  }
  
  // set values in object
  obj->SetValues(values);
}

/*--------------------------------------------------------------------------------*/
/** Parse attributes and subnodes as values
 *
 * @param obj object to read values from
 * @param userdata user suppled data
 */
/*--------------------------------------------------------------------------------*/
void TinyXMLADMData::ParseValues(ADMObject *obj, void *userdata)
{
  // if the supplied ADM object is an AudioChannelFormat, AudioBlockFormat sub-sections will be parsed
  ADMAudioChannelFormat *channel = dynamic_cast<ADMAudioChannelFormat *>(obj);
  const TiXmlNode       *node    = (const TiXmlNode *)userdata;
  const TiXmlNode       *subnode;

  // parse attributes
  ParseAttributes(obj, userdata);

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
          if ((block = new ADMAudioBlockFormat) != NULL)
          {
            ParseValues(obj->ToString() + ":BlockFormat", block, (void *)subnode);
            channel->Add(block);
          }
          else ERROR("Parsed object was not an AudioBlockFormat object");
        }
        else ERROR("No AudioChannelFormat for found AudioBlockFormat");
      }
      else ParseValue(obj, (void *)subnode);
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
