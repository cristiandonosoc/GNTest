// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/voxel_terrain.h"

#include "src/model/minecraft_cube.h"
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

  /* uint32_t offset = 2 * 4 * (uint32_t)face - (uint32_t)MinecraftCube::Face::kFront; */
  /* uvs->at(offset + 0) = uv_coords.bottom_left.x; */
  /* uvs->at(offset + 1) = uv_coords.bottom_left.y; */
  /* uvs->at(offset + 2) = uv_coords.top_right.x; */
  /* uvs->at(offset + 3) = uv_coords.bottom_left.y; */
  /* uvs->at(offset + 4) = uv_coords.top_right.x; */
  /* uvs->at(offset + 5) = uv_coords.top_right.y; */
  /* uvs->at(offset + 6) = uv_coords.bottom_left.x; */
  /* uvs->at(offset + 7) = uv_coords.top_right.y; */

  /* glBindBuffer(GL_ARRAY_BUFFER, vbo); */
  /* glBufferSubData(GL_ARRAY_BUFFER, */
  /*                 offset * sizeof(float), */
  /*                 8 * sizeof(float), */
  /*                 uvs->data() + offset); */
  /* glBindBuffer(GL_ARRAY_BUFFER, NULL); */
}

void SetCubeFace(const TextureAtlas& atlas,
                 MinecraftCube* cube,
                 MinecraftCube::Face face,
                 int texture_index1, int texture_index2) {
  ChangeUV(atlas, cube, face, texture_index1, 0);
  ChangeUV(atlas, cube, face, texture_index2, 0);
}

}  // namespace



VoxelTerrain::VoxelTerrain(TextureAtlas* atlas) : atlas_(atlas) {}

bool VoxelTerrain::Init() {
  // Create voxels.
  size_t amount = 10;
  for (size_t x = 0; x < amount; x++) {
    for(size_t z = 0; z < amount; z++) {
      MinecraftCube cube;
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
      cube.set_position({x, 0, z});
      terrain_.insert({x, z}, {});
    }
  }

  return true;
}

void VoxelTerrain::SetTextures(Shader* shader) const {
  atlas_->texture().Set(shader, GL_TEXTURE0);
  atlas_->texture().Set(shader, GL_TEXTURE1);
}

}  // namespace warhol
