// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <sstream>

#include "warhol/utils/location.h"

namespace warhol {

#ifndef NDEBUG
#define ASSERT(condition)                               \
  do {                                                  \
    if (!(condition)) {                                 \
      ::warhol::AssertionFailed(FROM_HERE, #condition); \
    }                                                   \
  } while (false)

#define NOT_IMPLEMENTED(message)                                      \
  do {                                                                \
    ::warhol::AssertionFailed(FROM_HERE, "Not implemented", message); \
  } while (false)

#define NOT_REACHED(message)                                       \
  do {                                                             \
    ::warhol::AssertionFailed(FROM_HERE, "Invalid path", message); \
  } while (false)

#else

#define ASSERT(condition) do {} while (false)
#define NOT_IMPLEMENTED() do {} while (false)
#define NOT_REACHED() do {} while (false)
#endif

void AssertionFailed(Location loc, const char* condition,
                     const char* msg = nullptr);

}  // namespace warhol
