#ifndef PNG_DECODER_H
#define PNG_DECODER_H

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <climits>
#include <cmath>

#include "Chunk.h"
#include "Endian.h"
#include "Inflate.h"

class PNG_Decoder {
private:
  std::filesystem::path fileName;
  unsigned int fileSize;
  char * bytes;
  int numChunks;
  std::vector<Chunk> chunks;
  
  // Private methods
  void LoadBytes();
  bool IsValid() const;
  void LoadChunks();
  static unsigned int GetNumChannels(unsigned char colorType);
  static void RemoveSubFilter(char * scanLine, unsigned int scanLineWidth, char * buffer, unsigned int bpp);
  static void RemoveUpFilter(char * scanLine, unsigned int scanLineWidth, char * buffer, char * priorScanLine = nullptr);
  static void RemoveAverageFilter(char * scanLine, unsigned int scanLineWidth, char * buffer, unsigned int bpp, char * priorScanLine = nullptr);
  static unsigned int PaethPredictor(int priorSub, int priorUp, int priorUpSub);
  static void RemovePaethFilter(char * scanLine, unsigned int scanLineWidth, char * buffer, unsigned int bpp, char * priorScanLine = nullptr);

public:
  // Constructors & Deconstructors
  PNG_Decoder();
  PNG_Decoder(std::filesystem::path fileName);
  ~PNG_Decoder();

  // Getters & Setters
  std::filesystem::path GetFile() const;
  char * GetBytes() const;
  int GetNumChunks() const;
  const std::vector<Chunk>& GetChunks() const;
  unsigned int GetWidth() const;
  unsigned int GetHeight() const;
  unsigned char GetBitDepth() const;
  unsigned char GetColorType() const;
  unsigned char GetCompressionMethod() const;
  unsigned char GetFilterMethod() const;
  unsigned char GetInterlaceMethod() const;

  // Methods
  void Open(const std::filesystem::path fileName);
  void Close();
  bool IsOpen() const;
  unsigned long AllocateCompressedData(char *& compressedData) const;
  static unsigned long AllocateDecompressedData(char * compressedData, unsigned long compressedDataSize, char *& decompressedData);
  static unsigned long AllocateUnfilteredData(char * decompressedData, char *& unfilteredData, unsigned int width,
    unsigned int height, unsigned char bitDepth, unsigned char colorType);
};

#endif