#include <iostream>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <chrono>

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

    uint64_t start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

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

    uint64_t end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    //std::cout << (end - start) << std::endl;


    std::cout << "const myCanvas = document.getElementById(\"myCanvas\");\nconst ctx = myCanvas.getContext(\"2d\");" << std::endl;
    for (unsigned int i = 0; i < decoder.GetHeight(); ++i) {
      for (unsigned int j = 0; j < decoder.GetWidth(); ++j) {
        unsigned int r = static_cast<unsigned int>(*reinterpret_cast<unsigned char *>(unfilteredData + (i * decoder.GetWidth() * 4) + (j * 4)));
        unsigned int g = static_cast<unsigned int>(*reinterpret_cast<unsigned char *>(unfilteredData + (i * decoder.GetWidth() * 4) + (j * 4) + 1));
        unsigned int b = static_cast<unsigned int>(*reinterpret_cast<unsigned char *>(unfilteredData + (i * decoder.GetWidth() * 4) + (j * 4) + 2));
        unsigned int a = static_cast<unsigned int>(*reinterpret_cast<unsigned char *>(unfilteredData + (i * decoder.GetWidth() * 4) + (j * 4) + 3));
        std::cout << "ctx.fillStyle = \"rgba(" << r << "," << g << "," << b << "," << a << ")\";" << std::endl;
        std::cout << "ctx.fillRect(" << j << "," << i << ", 1, 1);" << std::endl;
      }
    }


    std::free(compressedData);
    std::free(decompressedData);
    std::free(unfilteredData);
  } catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}