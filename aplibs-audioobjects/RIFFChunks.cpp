
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_LEVEL 1
#include <aplibs-dsp/ByteSwap.h>

#include "RIFFChunks.h"
#include "RIFFChunk_Definitions.h"
#include "RIFFFile.h"

/*--------------------------------------------------------------------------------*/
/** RIFF chunk - the first chunk of any WAVE file
 *
 * Don't read the data, no specific handling
 */
/*--------------------------------------------------------------------------------*/
void RIFFRIFFChunk::Register()
{
	RIFFChunk::RegisterProvider("RIFF", &Create);
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** WAVE chunk - specifies that the file contains WAV data
 *
 * This isn't actually a proper chunk - there is no length, just the ID, hence
 * a specific ReadChunk() function
 */
/*--------------------------------------------------------------------------------*/
bool RIFFWAVEChunk::ReadChunk(SoundFile *file)
{
	UNUSED_PARAMETER(file);

	// there is no data after WAVE to read
	return true;
}

void RIFFWAVEChunk::Register()
{
	RIFFChunk::RegisterProvider("WAVE", &Create);
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** fmt chunk - specifies the format of the WAVE data
 *
 * The chunk data is read, byte swapped and then processed
 *
 * Note this object is also derived from the SoundFormat class to allow generic
 * format handling facilities
 */
/*--------------------------------------------------------------------------------*/
void RIFFfmtChunk::Register()
{
	RIFFChunk::RegisterProvider("fmt ", &Create);
}

void RIFFfmtChunk::ByteSwapData()
{
	WAVEFORMAT_CHUNK& chunk = *(WAVEFORMAT_CHUNK *)data;

	if (SwapLittleEndian()) {
		BYTESWAP_VAR(chunk.Format);
		BYTESWAP_VAR(chunk.Channels);
		BYTESWAP_VAR(chunk.SampleRate);
		BYTESWAP_VAR(chunk.BytesPerSecond);
		BYTESWAP_VAR(chunk.BlockAlign);
		BYTESWAP_VAR(chunk.BitsPerSample);
	}
}

bool RIFFfmtChunk::ProcessChunkData()
{
	const WAVEFORMAT_CHUNK& chunk = *(const WAVEFORMAT_CHUNK *)data;
	bool success = false;

	if (chunk.Format == WAVE_FORMAT_PCM) {
		// cannot handle anything other that PCM samples

		DEBUG2(("Reading format data"));

		// set parameters within SoundFormat according to data from this chunk
		samplerate 	   = chunk.SampleRate;
		channels   	   = chunk.Channels;
		bytespersample = (chunk.BitsPerSample + 7) >> 3;

		// best guess at sample data format
		if (chunk.BitsPerSample <= 16) {
			format = SampleFormat_16bit;
		}
		else if (chunk.BitsPerSample <= 24) {
			format = SampleFormat_24bit;
		}
		else {
			format = SampleFormat_32bit;
		}

		// WAVE is always little-endian
		bigendian = false;

		success = true;
	}
	else ERROR("Format is %04x, not PCM", chunk.Format);

	return success;
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** bext chunk - specifies the Broadcast Extension data
 *
 * The chunk data is read, byte swapped but not processed
 *
 */
/*--------------------------------------------------------------------------------*/
void RIFFbextChunk::Register()
{
	RIFFChunk::RegisterProvider("bext", &Create);
}

void RIFFbextChunk::ByteSwapData()
{
	BROADCAST_CHUNK& chunk = *(BROADCAST_CHUNK *)data;

	if (SwapLittleEndian()) {
		BYTESWAP_VAR(chunk.TimeReferenceLow);
		BYTESWAP_VAR(chunk.TimeReferenceHigh);
		BYTESWAP_VAR(chunk.Version);
	}
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** chna chunk - part of the ADM (EBU Tech 3364)
 *
 * The chunk data is read, byte swapped but not processed (it is handled by the parent)
 *
 */
/*--------------------------------------------------------------------------------*/
void RIFFchnaChunk::Register()
{
	RIFFChunk::RegisterProvider("chna", &Create);
}

void RIFFchnaChunk::ByteSwapData()
{
	CHNA_CHUNK& chunk = *(CHNA_CHUNK *)data;

	if (SwapLittleEndian()) {
		BYTESWAP_VAR(chunk.TrackCount);
		BYTESWAP_VAR(chunk.UIDCount);

		uint16_t i;
		for (i = 0; i < chunk.UIDCount; i++) {
			BYTESWAP_VAR(chunk.UIDs[i].TrackNum);
		}
	}
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** axml chunk - part of the ADM (EBU Tech 3364)
 *
 * The chunk data is read, not byte swapped (it is pure XML) and not processed (it is handled by the parent)
 *
 */
/*--------------------------------------------------------------------------------*/
void RIFFaxmlChunk::Register()
{
	RIFFChunk::RegisterProvider("axml", &Create);
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** data chunk - WAVE data
 *
 * The data is not read (it could be too big to fit in memory)
 *
 * Note that the object is also derived from SoundfileSamples which provides sample
 * reading and conversion facilities
 *
 */
/*--------------------------------------------------------------------------------*/
bool RIFFdataChunk::ReadChunk(SoundFile *file)
{
	bool success = false;

	if (RIFFChunk::ReadChunk(file)) {
		// link file to SoundFileSamples object
		SetFile(file, datapos, length);

		success = true;
	}

	return success;
}

void RIFFdataChunk::Register()
{
	RIFFChunk::RegisterProvider("data", &Create);
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Register all providers from this file
 */
/*--------------------------------------------------------------------------------*/
void RegisterRIFFChunkProviders()
{
	RIFFRIFFChunk::Register();
	RIFFWAVEChunk::Register();
	RIFFfmtChunk::Register();
	RIFFbextChunk::Register();
	RIFFchnaChunk::Register();
	RIFFaxmlChunk::Register();
	RIFFdataChunk::Register();
}
