#include <iostream>
#include <string>
#include <filesystem>
#include <stdexcept>

#include "PNG_Decoder.h"

int main(int argc, char * argv[]) {
  try {
    if (argc != 2) {
      throw std::invalid_argument("./main <filename in media directory>");
    }

    const std::filesystem::path pngPath = argv[1];
    if (!std::filesystem::exists(pngPath)) {
      throw std::invalid_argument("'" + pngPath.string() + "' does not exist.");
    }

    PNG_Decoder decoder(pngPath);
    if (!decoder.IsOpen()) {
      throw std::runtime_error("Failed to open the provided PNG file.");
    }
    
    char * compressedData = nullptr;
    unsigned long compressedDataSize = decoder.AllocateCompressedData(compressedData);
    if (compressedDataSize == 0) {
      throw std::runtime_error("Failed to allocate compressed data.");
    }

    char * decompressedData = nullptr;
    unsigned long decompressedDataSize = PNG_Decoder::AllocateDecompressedData(compressedData, compressedDataSize, decompressedData);
    if (decompressedDataSize == 0) {
      throw std::runtime_error("Failed to allocate decompressed data.");
    }

    char * unfilteredData = nullptr;
    unsigned long unfilteredDataSize = PNG_Decoder::AllocateUnfilteredData(decompressedData, unfilteredData,
      decoder.GetWidth(), decoder.GetHeight(), decoder.GetBitDepth(), decoder.GetColorType());
    if (unfilteredDataSize == 0) {
      throw std::runtime_error("Failed to allocate unfiltered data.");
    }

    std::free(compressedData);
    std::free(decompressedData);
    std::free(unfilteredData);
  } catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}