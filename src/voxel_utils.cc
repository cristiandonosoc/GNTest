// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/voxel_utils.h"

#include "src/voxel_terrain.h"

namespace warhol {

void SetupSphere(VoxelTerrain* terrain, Vec3 center, int r) {
  Pair3<int> c = {(int)center.x, (int)center.y, (int)center.z};
  int r2 = r * r;
  for (int y = -r + 1; y < r; y++) {
    for (int z = -r + 1; z < r; z++) {
      for (int x = -r + 1; x < r; x++) {
        Vec3 p = {x, y, z};
        if (p.mag2() <= r2) {
          terrain->SetVoxel(c + Pair3<int>{x, y, z},
                            VoxelElement::Type::kGrassDirt);
        }
      }
    }
  }
}

}  // namespace warhol
