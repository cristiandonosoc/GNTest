// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/voxel_terrain.h"

#include "src/shader.h"
#include "src/texture_atlas.h"

namespace warhol {

namespace {

void
ChangeUV(const TextureAtlas& atlas,
         MinecraftCube* cube,
         MinecraftCube::Face face,
         int texture_index,
         int layer) {
  auto uv_coords = atlas.GetUVs(texture_index);
  cube->SetFace(face, layer, uv_coords);
}

void SetCubeFace(const TextureAtlas& atlas,
                 MinecraftCube* cube,
                 MinecraftCube::Face face,
                 int texture_index1, int texture_index2) {
  ChangeUV(atlas, cube, face, texture_index1, 0);
  ChangeUV(atlas, cube, face, texture_index2, 1);
}

}  // namespace



VoxelTerrain::VoxelTerrain(TextureAtlas* atlas) : atlas_(atlas) {}

bool VoxelTerrain::Init() {
  // Create voxels.
  int amount = 4;
  for (int x = 0; x < amount; x++) {
    for(int z = 0; z < amount; z++) {
      MinecraftCube cube;
      if (!cube.Init())
        return false;
      cube.set_position({x, 0, z});

      SetCubeFace(*atlas_, &cube, MinecraftCube::Face::kBack,
                  kGrassDirt, kTransparent);
      SetCubeFace(*atlas_, &cube, MinecraftCube::Face::kFront,
                  kGrassDirt, kTransparent);
      SetCubeFace(*atlas_, &cube, MinecraftCube::Face::kLeft,
                  kGrassDirt, kCrack4);
      SetCubeFace(*atlas_, &cube, MinecraftCube::Face::kRight,
                  kGrassDirt, kTransparent);
      SetCubeFace(*atlas_, &cube, MinecraftCube::Face::kTop,
                  kGrass, kCrack9);
      SetCubeFace(*atlas_, &cube, MinecraftCube::Face::kBottom,
                  kDirt, kTransparent);

      Pair<int> coord{x, z};
      terrain_[coord] = std::move(cube);
    }
  }
  return true;
}

void VoxelTerrain::SetTextures(Shader* shader) const {
  atlas_->texture().Set(shader, GL_TEXTURE0);
  atlas_->texture().Set(shader, GL_TEXTURE1);
}

void VoxelTerrain::Render(Shader* shader) {
  SetTextures(shader);
  for (auto& [coord, cube] : terrain_)
    cube.Render(shader);
}

}  // namespace warhol
