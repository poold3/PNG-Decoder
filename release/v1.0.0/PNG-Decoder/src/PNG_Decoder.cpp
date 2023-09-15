#include "PNG_Decoder.h"

// Private
void PNG_Decoder::LoadBytes() {
  std::ifstream inputStream;
  try {
    inputStream.open(this->fileName.string(), std::ifstream::binary);
    if (!inputStream.is_open()) {
      throw std::runtime_error("Failed to open file stream for '" + this->fileName.string() + "'.");
    }

    inputStream.seekg(0, std::ios_base::end);
    this->fileSize = static_cast<unsigned int>(inputStream.tellg());

    if (this->bytes == nullptr) {
      this->bytes = static_cast<char *>(std::malloc(this->fileSize * sizeof(char)));
    } else {
      this->bytes = static_cast<char *>(std::realloc(this->bytes, this->fileSize * sizeof(char)));
    }

    if (this->bytes == nullptr) {
      throw std::runtime_error("Failed to allocate memory to store the PNG data.");
    }

    inputStream.seekg(0, std::ios_base::beg);
    inputStream.read(this->bytes, this->fileSize);
    if (!inputStream.good()) {
      throw std::runtime_error("Failed to read in PNG file.");
    }
    inputStream.close();

    if (!this->IsValid()) {
      throw std::invalid_argument("'" + this->fileName.string() + "' is not a valid PNG.");
    }
  } catch(const std::exception& e) {
    this->fileSize = 0;
    std::free(this->bytes);
    this->bytes = nullptr;

    if (inputStream.is_open()) {
      inputStream.close();
    }
    std::cerr << e.what() << std::endl;
  }
}

bool PNG_Decoder::IsValid() const {
  try {
    // According to http://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html
    const unsigned char validBytes[] = {137, 80, 78, 71, 13, 10, 26, 10};
    for (int i = 0; i < 8; ++i) {
      if (validBytes[i] != static_cast<unsigned char>(this->bytes[i])) {
        throw std::invalid_argument("The first 8 bytes of this PNG do not match the standard.");
      }
    }

    if (static_cast<unsigned int>(this->GetCompressionMethod()) != 0) {
      throw std::invalid_argument("This PNG does not use the standard compression method.");
    }

    if (static_cast<unsigned int>(this->GetFilterMethod()) != 0) {
      throw std::invalid_argument("This PNG does not use the standard filter method.");
    }

    if (static_cast<unsigned int>(this->GetFilterMethod()) != 0) {
      throw std::invalid_argument("This decoder does not support the interlace method.");
    }

    return true;
  } catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

void PNG_Decoder::LoadChunks() {
  try {
    if (this->bytes == nullptr) {
      throw std::runtime_error("Failed to load chunks: PNG byte data is empty.");
    }
    this->numChunks = 0;
    this->chunks.resize(0);
    char * cur = this->bytes + 8;
    char * end = this->bytes + this->fileSize;

    while (cur < end) {
      Chunk curChunk = Chunk(cur);
      this->chunks.push_back(curChunk);
      this->numChunks += 1;
      cur += (static_cast<unsigned int>(12) + curChunk.GetDataLength());
    }

    if (this->chunks.at(0).GetChunkType() != ChunkType::IHDR || this->chunks.at(this->numChunks - 1).GetChunkType() != ChunkType::IEND) {
      throw std::invalid_argument("Invalid PNG chunk ordering.");
    }

  } catch(const std::exception& e) {
    this->numChunks = 0;
    this->chunks.resize(0);
    std::cerr << e.what() << std::endl;
  }
}

unsigned int PNG_Decoder::GetNumChannels(unsigned char colorType) {
  unsigned int type = static_cast<unsigned int>(colorType);
  if (type == 0) { // Grayscale
    return 1;
  } else if (type == 2) { // RGB triple
    return 3;
  } else if (type == 3) { // Palette index
    return 1;
  } else if (type == 4) { // Grayscale w/ alpha sample
    return 2;
  } else if (type == 6) { // RGB triple w/ alpha sample
    return 4;
  } else {
    return 0;
  }
}

void PNG_Decoder::RemoveSubFilter(char * scanLine, unsigned int scanLineWidth, char * buffer, unsigned int bpp) {
  for (unsigned int i = 0; i < scanLineWidth; ++i) {
    unsigned int prior = (bpp > i) ? 0 : static_cast<unsigned int>(*reinterpret_cast<unsigned char *>(buffer + (i - bpp)));
    unsigned int sub = static_cast<unsigned int>(*reinterpret_cast<unsigned char *>(scanLine + i));
    unsigned int raw = (sub + prior) % 256;
    buffer[i] = static_cast<char>(raw);
  }
}

void PNG_Decoder::RemoveUpFilter(char * scanLine, unsigned int scanLineWidth, char * buffer, char * priorScanLine) {
  for (unsigned int i = 0; i < scanLineWidth; ++i) {
    unsigned int prior = (priorScanLine == nullptr) ? 0 : static_cast<unsigned int>(*reinterpret_cast<unsigned char *>(priorScanLine + i));
    unsigned int up = static_cast<unsigned int>(*reinterpret_cast<unsigned char *>(scanLine + i));
    unsigned int raw = (up + prior) % 256;
    buffer[i] = static_cast<char>(raw);
  }
}

void PNG_Decoder::RemoveAverageFilter(char * scanLine, unsigned int scanLineWidth, char * buffer, unsigned int bpp, char * priorScanLine) {
  for (unsigned int i = 0; i < scanLineWidth; ++i) {
    unsigned int priorSub = (bpp > i) ? 0 : static_cast<unsigned int>(*reinterpret_cast<unsigned char *>(buffer + (i - bpp)));
    unsigned int priorUp = (priorScanLine == nullptr) ? 0 : static_cast<unsigned int>(*reinterpret_cast<unsigned char *>(priorScanLine + i));
    unsigned int average = static_cast<unsigned int>(*reinterpret_cast<unsigned char *>(scanLine + i));
    unsigned int raw = (average + ((priorSub + priorUp) / 2)) % 256;
    buffer[i] = static_cast<char>(raw);
  }
}

unsigned int PNG_Decoder::PaethPredictor(int priorSub, int priorUp, int priorUpSub) {
  int a = priorSub;
  int b = priorUp;
  int c = priorUpSub;
  int p = a + b - c;
  int pa = std::abs(p - a);
  int pb = std::abs(p - b);
  int pc = std::abs(p - c);
  if (pa == pb && pb == pc) {
    if (pa <= pb && pa <= pc) {
      return static_cast<unsigned int>(a);
    } else if (pb <= pc) {
      return static_cast<unsigned int>(b);
    } else {
      return static_cast<unsigned int>(c);
    }
  } else {
    int nearest = a;
    int smallest = pa;
    if (pb < smallest) {
      nearest = b;
      smallest = pb;
    }
    if (pc < smallest) {
      nearest = c;
    }
    return static_cast<unsigned int>(nearest);
  }
}

void PNG_Decoder::RemovePaethFilter(char * scanLine, unsigned int scanLineWidth, char * buffer, unsigned int bpp, char * priorScanLine) {
  for (unsigned int i = 0; i < scanLineWidth; ++i) {
    int priorSub = (bpp > i) ? 0 : static_cast<int>(*reinterpret_cast<unsigned char *>(buffer + (i - bpp)));
    int priorUp = (priorScanLine == nullptr) ? 0 : static_cast<int>(*reinterpret_cast<unsigned char *>(priorScanLine + i));
    int priorUpSub = (priorSub == 0 || priorUp == 0) ? 0 : static_cast<int>(*reinterpret_cast<unsigned char *>(priorScanLine + (i - bpp)));
    unsigned int paeth = static_cast<unsigned int>(*reinterpret_cast<unsigned char *>(scanLine + i));
    unsigned int raw = (paeth + PNG_Decoder::PaethPredictor(priorSub, priorUp, priorUpSub)) % 256;
    buffer[i] = static_cast<char>(raw);
  }
}

// Constructors & Deconstructors
PNG_Decoder::PNG_Decoder() {
  this->fileName = "";
  this->fileSize = 0;
  this->bytes = nullptr;
  this->numChunks = 0;
}

PNG_Decoder::PNG_Decoder(std::filesystem::path fileName) {
  this->fileName = fileName;
  this->bytes = nullptr;
  this->LoadBytes();
  this->LoadChunks();
}

PNG_Decoder::~PNG_Decoder() {
  this->Close();
}

// Getters
std::filesystem::path PNG_Decoder::GetFile() const {
  return this->fileName;
}

char * PNG_Decoder::GetBytes() const {
  return this->bytes;
}

int PNG_Decoder::GetNumChunks() const {
  return this->numChunks;
}

const std::vector<Chunk>& PNG_Decoder::GetChunks() const {
  return this->chunks;
}

unsigned int PNG_Decoder::GetWidth() const {
  if (this->bytes == nullptr) {
    throw std::runtime_error("No PNG file currently open.");
  }
  char * IhdrData = this->bytes + 16;
  unsigned int width = *reinterpret_cast<unsigned int *>(IhdrData);
  return Endian::ToHost(width);
}

unsigned int PNG_Decoder::GetHeight() const {
  if (this->bytes == nullptr) {
    throw std::runtime_error("No PNG file currently open.");
  }
  char * IhdrData = this->bytes + 16;
  unsigned int height = *reinterpret_cast<unsigned int *>(IhdrData + 4);
  return Endian::ToHost(height);
}

unsigned char PNG_Decoder::GetBitDepth() const {
  if (this->bytes == nullptr) {
    throw std::runtime_error("No PNG file currently open.");
  }
  char * IhdrData = this->bytes + 16;
  return * reinterpret_cast<unsigned char *>(IhdrData + 8);
}

unsigned char PNG_Decoder::GetColorType() const {
  if (this->bytes == nullptr) {
    throw std::runtime_error("No PNG file currently open.");
  }
  char * IhdrData = this->bytes + 16;
  return * reinterpret_cast<unsigned char *>(IhdrData + 9);
}

unsigned char PNG_Decoder::GetCompressionMethod() const {
  if (this->bytes == nullptr) {
    throw std::runtime_error("No PNG file currently open.");
  }
  char * IhdrData = this->bytes + 16;
  return * reinterpret_cast<unsigned char *>(IhdrData + 10);
}

unsigned char PNG_Decoder::GetFilterMethod() const {
  if (this->bytes == nullptr) {
    throw std::runtime_error("No PNG file currently open.");
  }
  char * IhdrData = this->bytes + 16;
  return * reinterpret_cast<unsigned char *>(IhdrData + 11);
}

unsigned char PNG_Decoder::GetInterlaceMethod() const {
  if (this->bytes == nullptr) {
    throw std::runtime_error("No PNG file currently open.");
  }
  char * IhdrData = this->bytes + 16;
  return * reinterpret_cast<unsigned char *>(IhdrData + 12);
}

// Methods
void PNG_Decoder::Open(const std::filesystem::path fileName) {
  this->fileName = fileName;
  this->LoadBytes();
  this->LoadChunks();
}

void PNG_Decoder::Close() {
  this->fileName = "";
  std::free(this->bytes);
  this->bytes = nullptr;
}

bool PNG_Decoder::IsOpen() const {
  return this->fileName.string().compare("") != 0 && this->bytes != nullptr && this->chunks.size() != 0;
}

unsigned long PNG_Decoder::AllocateCompressedData(char *& compressedData) const {
  try {
    if (!this->IsOpen()) {
      throw std::runtime_error("Failed to get data size because a PNG is not open.");
    }

    unsigned long compressedDataSize = 0;
    for (const Chunk& chunk: this->chunks) {
      if (chunk.GetChunkType() == ChunkType::IDAT) {
        compressedDataSize += chunk.GetDataLength();
      }
    }

    if (compressedData == nullptr) {
      compressedData = static_cast<char *>(std::malloc(compressedDataSize * sizeof(char)));
    } else {
      compressedData = static_cast<char *>(std::realloc(compressedData, compressedDataSize * sizeof(char)));
    }

    if (compressedData == nullptr) {
      throw std::runtime_error("Failed to allocate memory to store the compressed data.");
    }

    unsigned int currentIndex = 0;
    for (const Chunk& chunk: this->chunks) {
      if (chunk.GetChunkType() == ChunkType::IDAT) {
        std::copy(chunk.GetChunkData(), chunk.GetChunkData() + chunk.GetDataLength() + 1, compressedData + currentIndex);
        currentIndex += chunk.GetDataLength();
      }
    }
    return compressedDataSize;

  } catch(const std::exception& e) {
    std::free(compressedData);
    compressedData = nullptr;
    std::cerr << e.what() << std::endl;
    return 0;
  }
  
}

unsigned long PNG_Decoder::AllocateDecompressedData(char * compressedData, unsigned long compressedDataSize, char *& decompressedData) {
  try {
    if (compressedDataSize > UINT_MAX) {
      throw std::invalid_argument("Compressed data size is too large for this decoder.");
    }

    if (decompressedData == nullptr) {
      decompressedData = static_cast<char *>(std::malloc(UINT_MAX * sizeof(char)));
    } else {
      decompressedData = static_cast<char *>(std::realloc(decompressedData, UINT_MAX * sizeof(char)));
    }

    if (decompressedData == nullptr) {
      throw std::runtime_error("Failed to allocate memory to store the decompressed data.");
    }

    z_stream stream = Inflate::CreateZStream(compressedData, static_cast<unsigned int>(compressedDataSize), &decompressedData, UINT_MAX);
    Inflate::ZInflate(&stream, &decompressedData);
    return stream.total_out;

  } catch(const std::exception& e) {
    std::free(decompressedData);
    decompressedData = nullptr;
    std::cerr << e.what() << std::endl;
    return 0;
  }
}

unsigned long PNG_Decoder::AllocateUnfilteredData(char * decompressedData, char *& unfilteredData, unsigned int width,
  unsigned int height, unsigned char bitDepth, unsigned char colorType) {
  try {
    unsigned int numScanLines = height;
    unsigned int channels = PNG_Decoder::GetNumChannels(colorType);
    double exactScanLineWidth = (width * channels * static_cast<unsigned int>(bitDepth)) / 8.0;
    unsigned int scanLineWidth = static_cast<unsigned int>(std::ceil(exactScanLineWidth)); // In bytes, NOT INCLUDING FILTER BYTE
    unsigned int unfilteredDataSize = scanLineWidth * height; // In bytes
    unsigned int bpp = channels * static_cast<unsigned int>(bitDepth) / 8;
    bpp = (bpp == 0) ? 1 : bpp;

    if (unfilteredData == nullptr) {
      unfilteredData = static_cast<char *>(std::malloc(unfilteredDataSize * sizeof(char)));
    } else {
      unfilteredData = static_cast<char *>(std::realloc(unfilteredData, unfilteredDataSize * sizeof(char)));
    }

    for (unsigned int i = 0; i < numScanLines; ++i) {
      unsigned int currentIndex = i * (scanLineWidth + 1);
      unsigned int filterType = static_cast<unsigned int>(*reinterpret_cast<unsigned char *>(decompressedData + currentIndex));

      if (filterType == 0) { // None
        std::copy(decompressedData + currentIndex + 1, decompressedData + currentIndex + scanLineWidth + 2, unfilteredData + (i * scanLineWidth));
      } else if (filterType == 1) { // Sub
        PNG_Decoder::RemoveSubFilter(decompressedData + currentIndex + 1, scanLineWidth, unfilteredData + (i * scanLineWidth), bpp);
      }
      else if (filterType == 2) { // Up
        char * priorScanline = (i > 0) ? (unfilteredData + ((i - 1) * scanLineWidth)) : nullptr;
        PNG_Decoder::RemoveUpFilter(decompressedData + currentIndex + 1, scanLineWidth, unfilteredData + (i * scanLineWidth), priorScanline);
      }
      else if (filterType == 3) { // Average
        char * priorScanline = (i > 0) ? (unfilteredData + ((i - 1) * scanLineWidth)) : nullptr;
        PNG_Decoder::RemoveAverageFilter(decompressedData + currentIndex + 1, scanLineWidth, unfilteredData + (i * scanLineWidth), bpp, priorScanline);
      }
      else if (filterType == 4) { // Paeth
        char * priorScanline = (i > 0) ? (unfilteredData + ((i - 1) * scanLineWidth)) : nullptr;
        PNG_Decoder::RemovePaethFilter(decompressedData + currentIndex + 1, scanLineWidth, unfilteredData + (i * scanLineWidth), bpp, priorScanline);
      } else {
        throw std::invalid_argument("Invalid filter type.");
      }
    }

    return unfilteredDataSize;
  } catch(const std::exception& e) {
    std::free(unfilteredData);
    unfilteredData = nullptr;
    std::cerr << e.what() << std::endl;
    return 0;
  }
}
