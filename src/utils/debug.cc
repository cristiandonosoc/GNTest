// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdio.h>

#include "utils/debug.h"

namespace warhol {

void HexDump(const uint8_t* data, int length) {
  const uint8_t* ptr = data;
  const uint8_t* end = data + length;

  int index = 0;
  bool run = true;
  while (run) {
    // Row.
    printf("%08x ", index);
    for (size_t i = 0; i < 8; i++) {
      if (ptr >= end) {
        run = false;
        break;
      }

      printf("%04x ", *(const uint16_t*)ptr);
      ptr += 2;
    }
    index += 2*8;

    printf("\n");
  }
  printf("\n");
}

}  // namespace warhol
