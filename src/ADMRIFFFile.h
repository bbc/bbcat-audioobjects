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
  
  /*--------------------------------------------------------------------------------*/
  /** Open a WAVE/RIFF file
   *
   * @param filename filename of file to open
   *
   * @return true if file opened and interpreted correctly (including any extra chunks if present)
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool Open(const char *filename);

  /*--------------------------------------------------------------------------------*/
  /** Create empty ADM and populate basic track information
   *
   * @return true if successful
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool CreateADM();

  /*--------------------------------------------------------------------------------*/
  /** Create ADM from text file
   *
   * @param filename text filename (see below for format)
   *
   * @return true if successful
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
  virtual bool CreateADM(const char *filename);

  /*--------------------------------------------------------------------------------*/
  /** Close file
   *
   * @note this may take some time because it copies sample data from a temporary file
   */
  /*--------------------------------------------------------------------------------*/
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
