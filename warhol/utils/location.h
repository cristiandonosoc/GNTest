// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/utils/macros.h"

#include <utility>

namespace warhol {

#define FROM_HERE ::warhol::Location{__FILE__, __LINE__, __PRETTY_FUNCTION__}

// Location is meant to provide a thread local context about where the code
// is being called. Many logging operations will be scoped to this object.
struct Location {
  const char* file;
  int line;
  const char* function;
};

// Thread local location stacks.
struct LocationStack {
  static constexpr size_t kMaxLocationStackSize = 16;

  Location locations[kMaxLocationStackSize];
  int size = 0;
};

void PushLocation(Location);
void PopLocation();
LocationStack* GetLocationStack();
void PrintLocationStack(const LocationStack&);

#define SCOPE_LOCATION()                             \
  ::warhol::LocationBlock scope_location_##__LINE__( \
      {__FILE__, __LINE__, __PRETTY_FUNCTION__})

struct LocationBlock {
  LocationBlock(Location location) {
    PushLocation(std::move(location));
  }
  ~LocationBlock() {
    PopLocation();
  }
};

}  // namespace warhol
