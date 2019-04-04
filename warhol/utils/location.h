// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>
#include <utility>

#include "warhol/utils/macros.h"

namespace warhol {

#define FROM_HERE ::warhol::Location{__FILE__, __LINE__, __PRETTY_FUNCTION__}

// Location is meant to provide a thread local context about where the code
// is being called. Many logging operations will be scoped to this object.
struct Location {
  const char* file;
  int line;
  const char* function;
};

// Thread local location stacks ------------------------------------------------
//
// This permits to create an explicit call stack by using the SCOPE_LOCATION()
// macro. This stack is per-thread and can be queried by calling
// GetLocaitonStack. Assertions will query this stack to create a contextual
// stack trace. An actual stack trace would be better, but that would require
// to link libunwind (or equivalent) and I don't really want to do that (plus
// the windows compatibities issues).
//
// The macro is a no-op in optimized builds, so try to be liberal about its use.
//
// TODO: Add a context const char* so that the stack trace can have better
//       contextual info.

#ifndef NDEBUG
#define SCOPE_LOCATION()                             \
  ::warhol::LocationBlock scope_location_##__LINE__( \
      {__FILE__, __LINE__, __PRETTY_FUNCTION__})
#else
#define SCOPE_LOCATION()
#endif

struct LocationStack {
  static constexpr size_t kMaxLocationStackSize = 16;

  Location locations[kMaxLocationStackSize];
  int size = 0;
};

void PushLocation(Location);
void PopLocation();
LocationStack* GetLocationStack();
void PrintLocationStack(const LocationStack&);

struct LocationBlock {
  LocationBlock(Location location) {
    PushLocation(std::move(location));
  }

  ~LocationBlock() {
    PopLocation();
  }
};

}  // namespace warhol
