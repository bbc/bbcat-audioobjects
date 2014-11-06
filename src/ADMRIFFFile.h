#ifndef __ADM_RIFF_FILE__
#define __ADM_RIFF_FILE__

#include <string>
#include <map>

#include <bbcat-base/SelfRegisteringParametricObject.h>
#include <bbcat-base/PositionCursor.h>
#include <bbcat-base/ParameterSet.h>

#include "RIFFFile.h"
#include "ADMData.h"

BBC_AUDIOTOOLBOX_START

#define TYPE_RIFFFILE "rifffile"

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
  /** Set position of channel during writing
   *
   * @param channel channel to change the position of
   * @param pos new position
   * @param supplement optional extra information
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetPosition(uint_t channel, const Position& pos, const ParameterSet *supplement = NULL);

  /*--------------------------------------------------------------------------------*/
  /** Close file
   *
   * @note this may take some time because it copies sample data from a temporary file
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Close();

  /*--------------------------------------------------------------------------------*/
  /** Create cursors and add all objects to each cursor
   *
   * @note this can be called prior to writing samples or setting positions but it
   * @note *will* be called by SetPositions() if not done so already
   */
  /*--------------------------------------------------------------------------------*/
  virtual void PrepareCursors();

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
  std::vector<ADMTrackCursor *> cursors;        // *only* used during writing an ADM file
};

/*--------------------------------------------------------------------------------*/
/** *Very* thin class to provide a self-registering ADM file reader
 */
/*--------------------------------------------------------------------------------*/
class ADMFileReader : public ADMRIFFFile, public SelfRegisteringParametricObject
{
  SELF_REGISTER_CREATOR(ADMFileReader);

  virtual ~ADMFileReader() {}
};

BBC_AUDIOTOOLBOX_END

#endif
