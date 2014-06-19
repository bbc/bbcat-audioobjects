#ifndef __ADM_RIFF_FILE__
#define __ADM_RIFF_FILE__

#include <string>
#include <map>

#include "RIFFFile.h"
#include "ADMData.h"

BBC_AUDIOTOOLBOX_START

class ADMRIFFFile : public RIFFFile {
public:
	ADMRIFFFile();
	virtual ~ADMRIFFFile();
	
	virtual void Close();

	ADMData *GetADM() const {return adm;}

protected:
	virtual bool PostReadChunks();
	virtual void UpdateSamplePosition();

protected:
	ADMData *adm;
};

BBC_AUDIOTOOLBOX_END

#endif
