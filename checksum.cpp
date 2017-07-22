#include "checksum.h"

void checksum::add(BYTE value)
{
 BYTE cipher = (value ^ (r >> 8));
 r = (cipher + r) * c1 + c2;
 sum += cipher;
} // checksum::add(BYTE)
