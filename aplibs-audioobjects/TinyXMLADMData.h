#ifndef __TINY_XML_ADM_DATA__
#define __TINY_XML_ADM_DATA__

#include "ADMData.h"

/*--------------------------------------------------------------------------------*/
/** An implementation of ADMData using the TinyXML library (GPL)
 */
/*--------------------------------------------------------------------------------*/

class TiXmlNode;
class TinyXMLADMData : public ADMData {
public:
	TinyXMLADMData();
	virtual ~TinyXMLADMData();

	/*--------------------------------------------------------------------------------*/
	/** Register function - this MUST be called for this class to be usable
	 */
	/*--------------------------------------------------------------------------------*/
	static void Register() {ADMData::RegisterProvider(&__Creator);}

protected:
	/*--------------------------------------------------------------------------------*/
	/** Required implementation of XML translation
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool TranslateXML(const std::string& data);

	/*--------------------------------------------------------------------------------*/
	/** Required parsing functions
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void ParseHeader(ADMHEADER& header, const std::string& type, void *userdata);
	virtual void ParseValue(ADMObject *obj, const std::string& type, void *userdata);
	virtual void ParseValues(ADMObject *obj, const std::string& type, void *userdata);

	/*--------------------------------------------------------------------------------*/
	/** Find named element in subnode list
	 *
	 * @param node parent node
	 * @param name name of subnode to find
	 */
	/*--------------------------------------------------------------------------------*/
	virtual const TiXmlNode *FindElement(const TiXmlNode *node, const std::string& name);

	/*--------------------------------------------------------------------------------*/
	/** Recursively collect all objects in tree and parse them
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void CollectObjects(const TiXmlNode *node);

	/*--------------------------------------------------------------------------------*/
	/** Creator for this class
	 */
	/*--------------------------------------------------------------------------------*/
	static ADMData *__Creator(void *context) {
		UNUSED_PARAMETER(context);
		return new TinyXMLADMData();
	}
};

#endif
