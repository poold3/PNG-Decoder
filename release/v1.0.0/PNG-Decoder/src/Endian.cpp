#include "Endian.h"

ENDIAN_TYPES Endian::systemType = Endian::LoadEndian();

ENDIAN_TYPES Endian::LoadEndian() {
  unsigned int i = 1;
  unsigned char * c = reinterpret_cast<unsigned char *>(&i);
  if (*c) {
    return ENDIAN_TYPES::LITTLE;
  } else {
    return ENDIAN_TYPES::BIG;
  }
}

unsigned int Endian::ToHost(unsigned int i) {
  return Endian::systemType == ENDIAN_TYPES::LITTLE ? ntohl(i) : i;
}
