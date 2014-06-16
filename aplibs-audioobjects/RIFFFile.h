#ifndef __RIFF_FILE__
#define __RIFF_FILE__

#include <vector>
#include <map>

#include <aplibs-dsp/misc.h>

#include "RIFFChunks.h"

/*--------------------------------------------------------------------------------*/
/** 
 *
 * @param 
 *
 * @return 
 */
/*--------------------------------------------------------------------------------*/
class RIFFFile {
public:
	RIFFFile();
	virtual ~RIFFFile();

	virtual bool Open(const char *filename);
	bool IsOpen() const {return (file && (file->isopen()));}
	virtual void Close();

	SoundFile *GetFile() {return file;}

	enum {
		FileType_Unknown = 0,
		FileType_WAV,
		FileType_AIFF,
	};

	uint8_t	 	   GetFileType()       const {return filetype;}
	uint32_t 	   GetSampleRate() 	   const {return fileformat ? fileformat->GetSampleRate() : 0;}
	uint_t   	   GetChannels()   	   const {return fileformat ? fileformat->GetChannels() : 0;}
	uint_t   	   GetBytesPerSample() const {return fileformat ? fileformat->GetBytesPerSample() : 0;}
	SampleFormat_t GetSampleFormat()   const {return fileformat ? fileformat->GetSampleFormat() : SampleFormat_Unknown;}

	ulong_t		   GetPosition()	   const {return filesamples ? filesamples->GetPosition() : 0;}
	ulong_t		   GetLength()		   const {return filesamples ? filesamples->GetLength()   : 0;}

	virtual void   SetPosition(ulong_t pos) {if (filesamples) {filesamples->SetPosition(pos); UpdateSamplePosition();}}

	uint_t GetChunkCount() const {return chunklist.size();}
	RIFFChunk *GetChunkIndex(uint_t index) {return chunklist[index];}

	RIFFChunk *GetChunk(uint32_t id)  	   {return chunkmap[id];}
	RIFFChunk *GetChunk(const char *name)  {return GetChunk(IFFID(name));}

	SoundFileSamples *GetSamples()		   {return filesamples;}

	sint_t ReadFrames(uint8_t *buffer, SampleFormat_t type, uint_t nframes) {return filesamples ? filesamples->ReadSamples((uint8_t *)buffer, type, nframes) : -1;}
	sint_t ReadFrames(int16_t *buffer, uint_t nframes = 1) {return ReadFrames((uint8_t *)buffer, SampleFormatOf(buffer), nframes);}
	sint_t ReadFrames(int32_t *buffer, uint_t nframes = 1) {return ReadFrames((uint8_t *)buffer, SampleFormatOf(buffer), nframes);}
	sint_t ReadFrames(float   *buffer, uint_t nframes = 1) {return ReadFrames((uint8_t *)buffer, SampleFormatOf(buffer), nframes);}
	sint_t ReadFrames(double  *buffer, uint_t nframes = 1) {return ReadFrames((uint8_t *)buffer, SampleFormatOf(buffer), nframes);}

protected:
	virtual bool ReadChunks(ulong_t maxlength);
	virtual bool ProcessChunk(RIFFChunk *chunk) {(void)chunk; return true;}
	virtual bool PostReadChunks() {return true;}

	virtual void UpdateSamplePosition() {}

protected:
	SoundFile			  *file;
	uint8_t	 		  	  filetype;
	const SoundFormat *fileformat;
	SoundFileSamples      *filesamples;

	std::vector<RIFFChunk *>		chunklist;
	std::map<uint32_t, RIFFChunk *> chunkmap;
};

#endif
