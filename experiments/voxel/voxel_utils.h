// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/math/vec.h"

namespace warhol {

class VoxelTerrain;

// Set of utilities that make developing a voxel engine easier.

void SetupSphere(VoxelTerrain*, Vec3 center, int radius);

}  // namespace warhol
