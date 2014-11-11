#ifndef __ADM_FILE_READER__
#define __ADM_FILE_READER__

#include "ADMRIFFFile.h"

BBC_AUDIOTOOLBOX_START

#define TYPE_ADMBWF "admbwf"

/*--------------------------------------------------------------------------------*/
/** *Very* thin class to provide a self-registering ADM file reader
 */
/*--------------------------------------------------------------------------------*/
class ADMFileReader : public ADMRIFFFile, public SelfRegisteringParametricObject
{
  SELF_REGISTER_CREATOR(ADMFileReader);

  virtual ~ADMFileReader() {}

  /*--------------------------------------------------------------------------------*/
  /** Get a list of parameters for this object
   */
  /*--------------------------------------------------------------------------------*/
  static void GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list);
};

BBC_AUDIOTOOLBOX_END

#endif
