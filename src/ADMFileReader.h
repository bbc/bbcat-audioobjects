#ifndef __ADM_FILE_READER__
#define __ADM_FILE_READER__

#include "ADMRIFFFile.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** *Very* thin class to provide a self-registering ADM file reader
 */
/*--------------------------------------------------------------------------------*/
class ADMFileReader : public ADMRIFFFile, public SelfRegisteringParametricObject
{
public:
  ADMFileReader(const ParameterSet& parameters);
  virtual ~ADMFileReader() {}

  const std::string& GetADMObject() const {return admobject;}

  /*--------------------------------------------------------------------------------*/
  /** Get a list of parameters for this object
   */
  /*--------------------------------------------------------------------------------*/
  static void GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list);

protected:
  std::string admobject;
};

BBC_AUDIOTOOLBOX_END

#endif
