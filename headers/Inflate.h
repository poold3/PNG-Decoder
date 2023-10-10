#ifndef ZLIB_INFLATE_H
#define ZLIB_INFLATE_H

#include <stdexcept>
#include <iostream>
#include <string>
#include <climits>

#include "zlib.h"

class Inflate {
private:
  /* Reallocates *decompressed using std::realloc(*decompressed, size * sizeof(char)).
  Used by ZInflate() to allocate more memory to the zlib inflate() output if needed.
  After successful reallocation, the z_stream object's avail_out and next_out properties will be updated.
  The z_stream object will then be ready to be passed to zlib inflate() once again.*/
  static void ReallocDecompressed(z_stream * stream, char ** decompressed, unsigned long size);
public:
  static z_stream CreateZStream(char * compressed, unsigned int availableIn, char ** decompressed, unsigned int availableOut);
  static void ZInflate(z_stream * stream, char ** decompressed);
};

#endif