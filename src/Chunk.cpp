#include "Chunk.h"

Chunk::Chunk() {
  this->chunk = nullptr;
  this->dataLength = 0;
  this->chunkType = 0;
  this->chunkData = nullptr;
  this->crc = 0;
  this->ancillary = true;
}

Chunk::Chunk(char * chunk) {
  this->chunk = chunk;

  unsigned int length = *reinterpret_cast<unsigned int *>(chunk);
  this->dataLength = Endian::ToHost(length);

  unsigned int chunkType = *reinterpret_cast<unsigned int *>(chunk + 4);
  this->chunkType = Endian::ToHost(chunkType);

  this->chunkData = chunk + 8;

  unsigned int crc = *reinterpret_cast<unsigned int *>(chunk + 8 + this->dataLength);
  this->crc = Endian::ToHost(crc);

  unsigned int ancilliaryByte = (this->chunkType >> 28) & 1;
  this->ancillary = (ancilliaryByte == 1) ? true : false;
}

Chunk::~Chunk() {
  this->chunk = nullptr;
  this->chunkData = nullptr;
}

char * Chunk::GetChunk() const {
  return this->chunk;
}

unsigned int Chunk::GetDataLength() const {
  return this->dataLength;
}

unsigned int Chunk::GetChunkType() const {
  return this->chunkType;
}

char * Chunk::GetChunkData() const {
  return this->chunkData;
}

unsigned int Chunk::GetCrc() const {
  return this->crc;
}

bool Chunk::IsAncillary() const {
  return this->ancillary;
}
