// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/memory/memory_tracker.h"

#include <atomic>
#include <mutex>

#include "warhol/graphics/common/mesh.h"
#include "warhol/memory/memory_pool.h"
#include "warhol/utils/log.h"

namespace warhol {

// TrackToken ------------------------------------------------------------------

namespace {

void Clear(TrackToken* token) {
  token->type = TrackType::kLast;
  token->id = 0;
}

}  // namespace

TrackToken::~TrackToken() {
  if (!Valid(this))
    return;
  Untrack(this);
}

TrackToken::TrackToken(TrackToken&& other) {
  // If something is being tracker, it cannot move!
  ASSERT(!Valid(&other));

  type = other.type;
  id = other.id;
  Clear(&other);
}

TrackToken& TrackToken::operator=(TrackToken&& other) {
  if (this == &other)
    return *this;

  // If something is being tracker, it cannot move!
  ASSERT(!Valid(&other));

  type = other.type;
  id = other.id;
  Clear(&other);
  return *this;
}

// MemoryTracker ---------------------------------------------------------------


namespace {

std::mutex gMutex;

std::atomic<uint32_t> gNextMemoryPoolID = 1;

MemoryTracker gMemoryTracker;

}  // namespace

const MemoryTracker& GetGlobalTracker() {
  return gMemoryTracker;
}


void Track(MemoryPool* pool) {
  uint32_t id = gNextMemoryPoolID++;

  {
    std::lock_guard<std::mutex> lock(gMutex);
    gMemoryTracker.tracked_pools[id] = pool;
  }

  LOG(DEBUG) << "Tracking Memory Pool " << id;
  pool->track_token.id = id;
  pool->track_token.type = TrackType::kMemoryPool;
}

// Untrack

namespace {



}  // namespace

void Untrack(TrackToken* token) {
  LOG(DEBUG) << "Untracking "<< token->id;
  ASSERT(Valid(token));
  {
    std::lock_guard<std::mutex> lock(gMutex);
    switch (token->type) {
      case TrackType::kMemoryPool: {
        auto it = gMemoryTracker.tracked_pools.find(token->id);
        ASSERT(it != gMemoryTracker.tracked_pools.end());
        gMemoryTracker.tracked_pools.erase(it);
        break;
      }
      case TrackType::kLast:
        NOT_REACHED() << "Invalid track type: " << (uint32_t)token->type;
        break;
    }
  }

  Clear(token);
}

}  // namespace warhol
