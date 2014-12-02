
#define DEBUG_LEVEL 1
#include "ADMFileReader.h"

BBC_AUDIOTOOLBOX_START

SELF_REGISTERING_PARAMETRIC_OBJECT(ADMFileReader, TYPE_ADMBWF ".reader");

static const PARAMETERDESC _parameters[] = 
{
  {"filename", "Filename of ADM BWF file to read"},
  {"object",   "ADM object to playback"},
};

// MUST be in the same order as the above
enum
{
  Parameter_filename = 0,
  Parameter_object,
};

ADMFileReader::ADMFileReader(const ParameterSet& parameters) : ADMRIFFFile(),
                                                               admobject("all")
{
  std::string filename;

  if (parameters.Get(_parameters[Parameter_filename].name, filename))
  {
    if (!Open(filename.c_str())) InvalidateObject();
  }

  parameters.Get(_parameters[Parameter_object].name, admobject);

  SetParameters(parameters);
}

/*--------------------------------------------------------------------------------*/
/** Get a list of parameters for this object
 */
/*--------------------------------------------------------------------------------*/
void ADMFileReader::GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list)
{
  SelfRegisteringParametricObject::GetParameterDescriptions(list);

  AddParametersToList(_parameters, NUMBEROF(_parameters), list);
}

BBC_AUDIOTOOLBOX_END
