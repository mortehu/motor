#include <stdint.h>

#include "crc8.h"

unsigned char
crc8(const unsigned char* data, size_t len)
{
  unsigned char result = 0;

  while (len--)
    {
      uint_fast8_t v, i;

      v = *data++;

      for (i = 0; i < 8; ++i, v >>= 1)
        {
          unsigned char check = (result ^ v) & 0x01;

          result >>= 1;

          if (check)
            result ^= 0x8C;
        }
    }

  return result;
}
