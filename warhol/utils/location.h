// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <sstream>
#include <utility>

#include "warhol/utils/macros.h"

namespace warhol {

#define FROM_HERE ::warhol::Location{__FILE__, __LINE__, __FUNCTION__}

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

#ifdef DEBUG_MODE

#define SCOPE_LOCATION()                                                     \
  ::warhol::LocationScope STRINGIFY(__scope_location_, __LINE__)(FROM_HERE); \
  STRINGIFY(__scope_location_, __LINE__).stream()

#else

#define SCOPE_LOCATION() ::warhol::LocationNullStream().stream()

#endif

struct LocationStack;
struct LocationScope;
LocationStack* GetLocationStack();
std::string LocationStackToString(LocationStack*);

// Meant to be a scope trigger that will add elements to the stack.
struct LocationScope {
  LocationScope(Location location);
  ~LocationScope();
  DELETE_COPY_AND_ASSIGN(LocationScope);
  DELETE_MOVE_AND_ASSIGN(LocationScope);

  std::ostream& stream() { return stream_; }

  Location location;
  std::ostringstream stream_;
};

struct LocationStack {
  struct Entry {
    LocationScope* scope;
  };
  static constexpr size_t kMaxLocationStackSize = 16;

  Entry entries[kMaxLocationStackSize];
  int size = 0;
};

// Utility ---------------------------------------------------------------------

// This class is meant to look as a ostream, but ignore all input.
class LocationNullStream : std::ostream, std::streambuf {
 public:
   LocationNullStream() : std::ostream(this) {}

  // All operations are no-ops.
  int overflow(int) { return 0; }
  std::ostream& stream() { return *this; }
};

}  // namespace warhol
