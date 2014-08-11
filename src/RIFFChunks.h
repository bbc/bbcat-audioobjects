
#ifndef __RIFF_CHUNKS__
#define __RIFF_CHUNKS__

#include "RIFFChunk.h"
#include "SoundFileAttributes.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Chunk specific handling
 */
/*--------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** RIFF chunk - the first chunk of any WAVE file
 *
 * Don't read the data, no specific handling
 */
/*--------------------------------------------------------------------------------*/
class RIFFRIFFChunk : public RIFFChunk
{
public:
  RIFFRIFFChunk(uint32_t chunk_id) : RIFFChunk(chunk_id) {}
  virtual ~RIFFRIFFChunk() {}

  // provider function register for this object
  static void Register();

protected:
  // provider function for this object
  static RIFFChunk *Create(uint32_t id, void *context)
  {
    (void)context;
    return new RIFFRIFFChunk(id);
  }

protected:
  // do NOT read OR skip over data
  virtual ChunkHandling_t GetChunkHandling() const {return ChunkHandling_RemainInChunkData;}
  // just return true - no data to write
  virtual bool WriteChunkData(SoundFile *file);
};

/*--------------------------------------------------------------------------------*/
/** WAVE chunk - specifies that the file contains WAV data
 *
 * This isn't actually a proper chunk - there is no length, just the ID, hence
 * a specific ReadChunk() function
 */
/*--------------------------------------------------------------------------------*/
class RIFFWAVEChunk : public RIFFChunk
{
public:
  RIFFWAVEChunk(uint32_t chunk_id) : RIFFChunk(chunk_id) {}
  virtual ~RIFFWAVEChunk() {}

  // special write chunk function (no length or data)
  virtual bool WriteChunk(SoundFile *file);

  // provider function register for this object
  static void Register();

protected:
  // provider function for this object
  static RIFFChunk *Create(uint32_t id, void *context)
  {
    (void)context;
    return new RIFFWAVEChunk(id);
  }

protected:
  // dummy read function
  virtual bool ReadChunk(SoundFile *file);
};

/*--------------------------------------------------------------------------------*/
/** fmt chunk - specifies the format of the WAVE data
 *
 * The chunk data is read, byte swapped and then processed
 *
 * Note this object is also derived from the SoundFormat class to allow generic
 * format handling facilities
 */
/*--------------------------------------------------------------------------------*/
class RIFFfmtChunk : public RIFFChunk, public SoundFormat
{
public:
  RIFFfmtChunk(uint32_t chunk_id) : RIFFChunk(chunk_id),
                                    SoundFormat() {}
  virtual ~RIFFfmtChunk() {}

  // create write data
  virtual bool CreateWriteData();

  // provider function register for this object
  static void Register();

protected:
  // provider function for this object
  static RIFFChunk *Create(uint32_t id, void *context)
  {
    (void)context;
    return new RIFFfmtChunk(id);
  }

protected:
  // data will need byte swapping on certain machines
  virtual void ByteSwapData();
  // always read and process his kind of chunk
  virtual ChunkHandling_t GetChunkHandling() const {return ChunkHandling_ReadChunk;}
  // chunk processing
  virtual bool ProcessChunkData();
  // delete data after processing
  virtual bool DeleteDataAfterProcessing() const {return true;}
};

/*--------------------------------------------------------------------------------*/
/** bext chunk - specifies the Broadcast Extension data
 *
 * The chunk data is read, byte swapped but not processed
 *
 */
/*--------------------------------------------------------------------------------*/
class RIFFbextChunk : public RIFFChunk
{
public:
  RIFFbextChunk(uint32_t chunk_id) : RIFFChunk(chunk_id) {}
  virtual ~RIFFbextChunk() {}

  // create write data
  virtual bool CreateWriteData();

  // provider function register for this object
  static void Register();

protected:
  // provider function for this object
  static RIFFChunk *Create(uint32_t id, void *context)
  {
    (void)context;
    return new RIFFbextChunk(id);
  }

protected:
  // data will need byte swapping on certain machines
  virtual void ByteSwapData();
  // data should be read
  virtual ChunkHandling_t GetChunkHandling() const {return ChunkHandling_ReadChunk;}
};

/*--------------------------------------------------------------------------------*/
/** chna chunk - part of the ADM (EBU Tech 3364)
 *
 * The chunk data is read, byte swapped but not processed (it is handled by the parent)
 *
 */
/*--------------------------------------------------------------------------------*/
class RIFFchnaChunk : public RIFFChunk
{
public:
  RIFFchnaChunk(uint32_t chunk_id) : RIFFChunk(chunk_id) {}
  virtual ~RIFFchnaChunk() {}

  // provider function register for this object
  static void Register();

protected:
  // provider function for this object
  static RIFFChunk *Create(uint32_t id, void *context)
  {
    (void)context;
    return new RIFFchnaChunk(id);
  }

protected:
  // byte swapping required
  virtual void ByteSwapData();
  // data should be read
  virtual ChunkHandling_t GetChunkHandling() const {return ChunkHandling_ReadChunk;}
};

/*--------------------------------------------------------------------------------*/
/** axml chunk - part of the ADM (EBU Tech 3364)
 *
 * The chunk data is read, not byte swapped (it is pure XML) and not processed (it is handled by the parent)
 *
 */
/*--------------------------------------------------------------------------------*/
class RIFFaxmlChunk : public RIFFChunk
{
public:
  RIFFaxmlChunk(uint32_t chunk_id) : RIFFChunk(chunk_id) {}
  virtual ~RIFFaxmlChunk() {}

  // provider function register for this object
  static void Register();

protected:
  // provider function for this object
  static RIFFChunk *Create(uint32_t id, void *context)
  {
    (void)context;
    return new RIFFaxmlChunk(id);
  }

protected:
  // data should be read
  virtual ChunkHandling_t GetChunkHandling() const {return ChunkHandling_ReadChunk;}
};

/*--------------------------------------------------------------------------------*/
/** data chunk - WAVE data
 *
 * The data is not read (it could be too big to fit in memory)
 *
 * Note that the object is also derived from SoundFileSamples which provides sample
 * reading and conversion facilities
 *
 */
/*--------------------------------------------------------------------------------*/
class RIFFdataChunk : public RIFFChunk, public SoundFileSamples
{
public:
  RIFFdataChunk(uint32_t chunk_id) : RIFFChunk(chunk_id),
                                     SoundFileSamples() {}
  virtual ~RIFFdataChunk();

  // set up data length before data is written
  virtual bool CreateWriteData();

  // provider function register for this object
  static void Register();

protected:
  // provider function for this object
  static RIFFChunk *Create(uint32_t id, void *context)
  {
    (void)context;
    return new RIFFdataChunk(id);
  }

protected:
  // initialise chunk when writing a file
  virtual bool InitialiseForWriting();
  // perform additional initialisation after chunk read
  virtual bool ReadChunk(SoundFile *file);
  // copy sample data from temporary file
  virtual bool WriteChunkData(SoundFile *file);
};

/*--------------------------------------------------------------------------------*/
/** Register all providers from this file
 */
/*--------------------------------------------------------------------------------*/
extern void RegisterRIFFChunkProviders();

BBC_AUDIOTOOLBOX_END

#endif
