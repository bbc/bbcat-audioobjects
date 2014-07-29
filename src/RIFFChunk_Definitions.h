#ifndef __RIFF_CHUNK_DEFINITIONS__
#define __RIFF_CHUNK_DEFINITIONS__

#include <aplibs-dsp/misc.h>

BBC_AUDIOTOOLBOX_START

typedef PACKEDSTRUCT
{
  uint8_t Count;
  uint8_t String[1];
} PSTRING;

#define RIFF_ID IFFID("RIFF")
#define WAVE_ID IFFID("WAVE")

typedef PACKEDSTRUCT
{
  uint16_t     NChannels;
  uint32_t     SampleFrames;
  uint16_t     SampleSize;
  IEEEEXTENDED SampleRate;
} COMM_CHUNK;

#define COMM_ID IFFID("COMM")

typedef PACKEDSTRUCT
{
  uint16_t     NChannels;
  uint32_t     SampleFrames;
  uint16_t     SampleSize;
  IEEEEXTENDED SampleRate;
  uint32_t     CompressionID;
  PSTRING      CompressionString;
  uint8_t      __pad[254];
} COMM_CHUNK_EX;

typedef PACKEDSTRUCT
{
  uint32_t     TimestampHigh;
  uint32_t     TimestampLow;
} TMST_CHUNK;

#define TMST_ID IFFID("TMST")

typedef PACKEDSTRUCT
{
  uint32_t     Offset;
  uint32_t     BlockSize;
} SSND_CHUNK;

#define SSND_ID IFFID("SSND")

typedef PACKEDSTRUCT
{
  char         Description[256];                      /* ASCII : «Description of the sound sequence» */
  char         Originator[32];                        /* ASCII : «Name of the originator» */
  char         OriginatorReference[32];               /* ASCII : «Reference of the originator» */
  char         OriginationDate[10];                   /* ASCII : «yyyy:mm:dd» */
  char         OriginationTime[8];                    /* ASCII : «hh:mm:ss» */
  uint32_t     TimeReferenceLow;                      /* First sample count since midnight, low word */
  uint32_t     TimeReferenceHigh;                     /* First sample count since midnight, high word */
  uint16_t     Version;                               /* Version of the BWF; unsigned binary number */
  char         Reserved[254];                         /* Reserved for future use, set to 'NULL' */
  char         CodingHistory[0];                      /* ASCII : «History coding » */
} BROADCAST_CHUNK;

#define bext_ID IFFID("bext")

typedef PACKEDSTRUCT
{
  uint16_t     Format;
  uint16_t     Channels;
  uint32_t     SampleRate;
  uint32_t     BytesPerSecond;
  uint16_t     BlockAlign;
  uint16_t     BitsPerSample;
} WAVEFORMAT_CHUNK;

#define WAVE_FORMAT_PCM 0x0001
#define fmt_ID IFFID("fmt ")

#define data_ID IFFID("data")

#define chna_ID IFFID("chna")
typedef PACKEDSTRUCT
{
  uint16_t    TrackCount;
  uint16_t    UIDCount;
  PACKEDSTRUCT
  {
    uint16_t TrackNum;
    char     UID[12];
    char     TrackRef[14];
    char     PackRef[11];
    uint8_t  _pad;
  } UIDs[0];
} CHNA_CHUNK;

#define axml_ID IFFID("axml")

BBC_AUDIOTOOLBOX_END

#endif
