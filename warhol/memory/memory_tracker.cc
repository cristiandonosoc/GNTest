// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/memory/memory_tracker.h"

#include "warhol/graphics/common/mesh.h"
#include "warhol/memory/memory_pool.h"
#include "warhol/utils/assert.h"

namespace warhol {

// MemoryPool ------------------------------------------------------------------

template <>
void Track<MemoryPool>(MemoryTracker* tracker, MemoryPool* pool) {
  ASSERT(!Active(&pool->track_token));
  tracker->tracked_pools.insert(pool);
  pool->track_token.tracker = tracker;
  pool->track_token.tracked = pool;
}

template <>
void Untrack<MemoryPool>(MemoryTracker* tracker, MemoryPool* pool) {
  ASSERT(Active(&pool->track_token));
  auto it = tracker->tracked_pools.find(pool);
  ASSERT(it != tracker->tracked_pools.end());

  tracker->tracked_pools.erase(it);
  Clear(&pool->track_token);
}

// Mesh ------------------------------------------------------------------------

template <>
void Track<Mesh>(MemoryTracker* tracker, Mesh* mesh) {
  ASSERT(!Active(&mesh->track_token));
  tracker->tracked_meshes.insert(mesh);

  mesh->track_token.tracker = tracker;
  mesh->track_token.tracked = mesh;
}

template <>
void Untrack<Mesh>(MemoryTracker* tracker, Mesh* mesh) {
  ASSERT(Active(&mesh->track_token));
  auto it = tracker->tracked_meshes.find(mesh);
  ASSERT(it != tracker->tracked_meshes.end());

  tracker->tracked_meshes.erase(it);
  Clear(&mesh->track_token);
}

}  // namespace warhol
