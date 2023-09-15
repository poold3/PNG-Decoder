#ifndef CHUNK_H
#define CHUNK_H

#include <iostream>
#include <string>
#include <stdexcept>

#include "Endian.h"

struct ChunkType {
public:
  static const unsigned int IHDR = 1229472850;
  static const unsigned int PLTE = 1347179589;
  static const unsigned int IDAT = 1229209940;
  static const unsigned int IEND = 1229278788;
};

class Chunk {
private:
  char * chunk;
  unsigned int dataLength;
  unsigned int chunkType;
  char * chunkData;
  unsigned int crc;
  bool ancillary;

public:
  Chunk();
  Chunk(char * chunk);
  ~Chunk();

  char * GetChunk() const;
  unsigned int GetDataLength() const;
  unsigned int GetChunkType() const;
  char * GetChunkData() const;
  unsigned int GetCrc() const;
  bool IsAncillary() const;

};


#endif