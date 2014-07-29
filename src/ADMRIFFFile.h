#ifndef __ADM_RIFF_FILE__
#define __ADM_RIFF_FILE__

#include <string>
#include <map>

#include "RIFFFile.h"
#include "ADMData.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** ADM BWF file support class
 *
 * This class uses an external interpretation mechanism to interpret the ADM data
 * contained within an BWF file to create an ADMData object which is then accessible
 * from this object
 *
 * The ADMData class does NOT provide any interpretation implementation but does provide
 * ADM data manipulation and chunk generation
 *
 * An example implementation of the interpretation is in TinyXMLADMData.h
 */
/*--------------------------------------------------------------------------------*/
class ADMRIFFFile : public RIFFFile
{
public:
  ADMRIFFFile();
  virtual ~ADMRIFFFile();
  
  virtual void Close();

  ADMData *GetADM() const {return adm;}

protected:
  /*--------------------------------------------------------------------------------*/
  /** Post processing function - actually performs the interpretation of the ADM once
   * it has been read
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool PostReadChunks();

  virtual void UpdateSamplePosition();

protected:
  ADMData *adm;
};

BBC_AUDIOTOOLBOX_END

#endif
