#ifndef ENDIAN_H
#define ENDIAN_H

#include <arpa/inet.h>

enum ENDIAN_TYPES {
  BIG,
  LITTLE
};

class Endian {
public:
  static ENDIAN_TYPES systemType;
  static ENDIAN_TYPES LoadEndian();
  static unsigned int ToHost(unsigned int i);
};
#endif
