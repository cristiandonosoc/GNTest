// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "src/math/vec.h"

namespace warhol {

class Camera;

// Set of debug primitives we can add to draw elements to draw.
class DebugVolumes {
 public:

  // |radius| is how much the center will extend.
  static void AABB(Vec3 center, Vec3 radius, Vec3 color = {1, 0, 0});

  static void NewFrame();
  static void RenderVolumes(Camera*);

};

}  // namespace warhol
