// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "warhol/utils/location.h"

namespace warhol {

#ifndef NDEBUG
#define ASSERT(condition)                               \
  do {                                                  \
    if (!(condition)) {                                 \
      ::warhol::AssertionFailed(FROM_HERE, #condition); \
    }                                                   \
  } while (false)

#define NOT_IMPLEMENTED()                                    \
  do {                                                       \
    ::warhol::AssertionFailed(FROM_HERE, "Not implemented"); \
  } while (false)

#else

#define ASSERT(condition) \
  do {                    \
  } while (false)
#define NOT_IMPLEMENTED() \
  do {                    \
  } while (false)
#endif

void AssertionFailed(Location loc, const char* condition);

}  // namespace warhol
