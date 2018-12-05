// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "src/utils/clear_on_move.h"
#include "src/utils/macros.h"

namespace warhol {

#define FROM_HERE ::warhol::Location{__FILE__, __LINE__}

#define FROM_HERE_SCOPE() \
  ::warhol::LocationTrigger loc_trigger##__LINE__(FROM_HERE)

// Location is meant to provide a thread local context about where the code
// is being called. Many logging operations will be scoped to this object.
struct Location {
  // If the thread-local location it is returned. Otherwise return the given
  // one.
  static Location GetThreadCurrentLocation(const Location&);

  bool valid() const { return !!file; }

  const char* file;
  int line;
};

class LocationTrigger {
 public:
  LocationTrigger(Location);
  ~LocationTrigger();

  DELETE_COPY_AND_ASSIGN(LocationTrigger);
  DELETE_MOVE_AND_ASSIGN(LocationTrigger);

 private:
  Location location_;
};

}  // namespace warhol
