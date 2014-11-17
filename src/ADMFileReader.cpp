
#define DEBUG_LEVEL 1
#include "ADMFileReader.h"

BBC_AUDIOTOOLBOX_START

static const struct {
  PARAMETERDESC filename;
  PARAMETERDESC object;
} _parameters = 
{
  {"filename", "Filename of ADM BWF file to read"},
  {"object",   "ADM object to playback"},
};

SELF_REGISTERING_PARAMETRIC_OBJECT(ADMFileReader, TYPE_ADMBWF ".reader");

ADMFileReader::ADMFileReader(const ParameterSet& parameters) : ADMRIFFFile(),
                                                               admobject("all")
{
  std::string filename;

  if (parameters.Get(_parameters.filename.name, filename))
  {
    Open(filename.c_str());
  }

  parameters.Get(_parameters.object.name, admobject);

  SetParameters(parameters);
}

/*--------------------------------------------------------------------------------*/
/** Get a list of parameters for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMFileReader::GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list)
{
  const PARAMETERDESC *pparameters = (const PARAMETERDESC *)&_parameters;
  uint_t i, n = sizeof(_parameters) / sizeof(pparameters[0]);

  SelfRegisteringParametricObject::GetParameterDescriptions(list);

  for (i = 0; i < n; i++) list.push_back(pparameters + i);
}

BBC_AUDIOTOOLBOX_END
