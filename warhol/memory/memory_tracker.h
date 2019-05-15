// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <map>

#include "warhol/utils/log.h"

namespace warhol {

struct MemoryPool;
struct Mesh;

enum class TrackType : uint32_t {
  kMemoryPool,
  kLast,
};
const char* ToString(TrackType);

struct TrackToken {
  TrackToken() = default;
  ~TrackToken();
  DELETE_COPY_AND_ASSIGN(TrackToken);
  DECLARE_MOVE_AND_ASSIGN(TrackToken);

  TrackType type = TrackType::kLast;
  uint32_t id = 0;
};
inline bool
Valid(TrackToken* t) { return t->type != TrackType::kLast && t->id != 0; }

// Memory Tracker --------------------------------------------------------------

struct MemoryTracker {
  std::map<uint32_t, MemoryPool*> tracked_pools;
};

const MemoryTracker& GetGlobalTracker();

// Track will be specialized in the .cc
void Track(MemoryPool*);

void Untrack(TrackToken*);

}  // namespace warhol
