// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <set>

#include "warhol/utils/assert.h"
#include "warhol/utils/macros.h"

namespace warhol {

struct MemoryPool;
struct Mesh;

struct MemoryTracker {
  std::set<MemoryPool*> tracked_pools;
  std::set<Mesh*> tracked_meshes;
};

// Track will be specialized in the .cc
template <typename T>
void Track(MemoryTracker*, T*);

template <typename T>
void Untrack(MemoryTracker*, T*);


// Each object being tracked will have an active token.
// When active, this object cannot *move* (but can be moved to).
// On destruction, the token will untrack itself if active.
//
// IMPORTANT: The tracker *must* outlive all track tokens. This is typically
//            done by having the MemoryTracker being the last object to go away
//            in the program.
template <typename T>
struct MemoryTrackToken {
  MemoryTrackToken() = default;
  ~MemoryTrackToken() {
    Untrack();
  }

  DELETE_COPY_AND_ASSIGN(MemoryTrackToken);

  MemoryTrackToken(MemoryTrackToken&& other) {
    ASSERT(!other.tracker);
    Untrack();
    other.Clear();
  }

  MemoryTrackToken& operator=(MemoryTrackToken&& other) {
    ASSERT(!other.tracker);
    Untrack();
    other.Clear();
    return *this;
  }

  void Untrack() {
    if (!tracker)
      return;
    ASSERT(tracked);
    ::warhol::Untrack(tracker, tracked);
    Clear();
  }

  void Clear() {
    tracker = nullptr;
    tracked = nullptr;
  }

  // When active, this is not-null.
  MemoryTracker* tracker = nullptr;
  T* tracked = nullptr;
};

template <typename T>
inline bool Active(MemoryTrackToken<T>* token) { return !!token->tracker; }

template <typename T>
inline void Clear(MemoryTrackToken<T>* token) {
  token->tracker = nullptr;
  token->tracked = nullptr;
}

}  // namespace warhol
