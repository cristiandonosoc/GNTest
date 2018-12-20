// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/debug/volumes.h"

#include <vector>

#include "src/utils/macros.h"

namespace warhol {

namespace {

struct DebugAABB {
  Vec3 center;
  Vec3 radius;
  Vec3 color;
};

struct DebugVolumesHolder {
  static DebugVolumesHolder& get() {
    static DebugVolumesHolder holder;
    return holder;
  }

  std::vector<DebugAABB> aabbs;

 private:
  DebugVolumesHolder() = default;
  DELETE_COPY_AND_ASSIGN(DebugVolumesHolder);
  DELETE_MOVE_AND_ASSIGN(DebugVolumesHolder);
};

// Render functions ------------------------------------------------------------

void
RenderAABB(const DebugAABB&) {}

}  // namespace

void DebugVolumes::AABB(Vec3 center, Vec3 radius, Vec3 color) {
  DebugAABB aabb = {};
  aabb.center = center;
  aabb.radius = radius;
  aabb.color = color;
  DebugVolumesHolder::get().aabbs.push_back(std::move(aabb));
}

void DebugVolumes::NewFrame() {
  DebugVolumesHolder::get().aabbs.clear();
}

void DebugVolumes::RenderVolumes() {
  // AABBs
  auto& holder = DebugVolumesHolder::get();
  for (auto& aabb : holder.aabbs) {
    RenderAABB(aabb);
  }
}

}  // namespace warhol
