// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/utils/types.h"

#include "warhol/utils/string.h"

namespace warhol {

std::string BytesToString(size_t bytes) {
  if (bytes < KILOBYTES(1))
    return StringPrintf("%lu bytes", bytes);
  if (bytes < MEGABYTES(1))
    return StringPrintf("%f KBs", (float)bytes / (float)KILOBYTES(1));
  if (bytes < GIGABYTES(1))
    return StringPrintf("%f MBs", (float)bytes / (float)MEGABYTES(1));
  return StringPrintf("%f GBs", (float)bytes / (float)GIGABYTES(1));
}

}  // namespace warhol
