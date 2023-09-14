#include "Inflate.h"

z_stream Inflate::CreateZStream(char * compressed, unsigned int availableIn, char ** decompressed, unsigned int availableOut) {
  z_stream stream;
  stream.next_in = reinterpret_cast<Bytef *>(compressed);
  stream.avail_in = availableIn;
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;
  stream.avail_out = availableOut;
  stream.next_out = reinterpret_cast<Bytef *>(*decompressed);
  return stream;
}

void Inflate::ZInflate(z_stream * stream, char ** decompressed) {
  if (inflateInit(stream) != Z_OK) {
    std::string msg = stream->msg;
    throw std::runtime_error("InflateInit failed: " + msg);
  }

  int inflateStatus = Z_OK;
  while (inflateStatus != Z_STREAM_END && stream->avail_out > 0) {
    inflateStatus = inflate(stream, Z_SYNC_FLUSH);
    if (inflateStatus != Z_OK && inflateStatus != Z_STREAM_END) {
      std::string msg = "Inflate failed: ";
      if (stream->msg) {
        msg.append(stream->msg);
      } else {
        msg.append(std::to_string(inflateStatus));
      }
      throw std::runtime_error(msg);
    } else if (inflateStatus == Z_OK && stream->avail_out == 0) {
      ReallocDecompressed(stream, decompressed, stream->total_out + UINT_MAX);
    }
  }

  *decompressed = static_cast<char *>(std::realloc(*decompressed, stream->total_out * sizeof(char)));
  if (*decompressed == nullptr) {
    throw std::runtime_error("Failed to reallocate memory to store the decompressed data stream.");
  }
}

void Inflate::ReallocDecompressed(z_stream * stream, char ** decompressed, unsigned long size) {
  *decompressed = static_cast<char *>(std::realloc(*decompressed, size * sizeof(char)));
  if (*decompressed == nullptr) {
    throw std::runtime_error("Failed to reallocate memory to store the decompressed data stream.");
  }
  stream->avail_out = size - stream->total_out;
  stream->next_out = reinterpret_cast<Bytef *>((*decompressed) + stream->total_out);
}
