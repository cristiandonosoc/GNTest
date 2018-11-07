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

// VoxelChunk ------------------------------------------------------------------

VoxelChunk::VoxelChunk() = default;
VoxelChunk::VoxelChunk(TextureAtlas* atlas) : atlas_(atlas) {}

MinecraftCube& VoxelChunk::GetVoxel(size_t x, size_t y, size_t z) {
  assert(x < kVoxelChunkSize && y < kVoxelChunkSize && z < kVoxelChunkSize);
  size_t z_offset = z * kVoxelChunkSize * kVoxelChunkSize;
  size_t y_offset = y * kVoxelChunkSize;
  return voxels_[z_offset + y_offset + x];
}

bool VoxelChunk::Init() {
  for (size_t x = 0; x < kVoxelChunkSize; x++) {
    for (size_t y = 0; y < kVoxelChunkSize; y++) {
      for (size_t z = 0; z < kVoxelChunkSize; z++) {
        MinecraftCube& voxel = GetVoxel(x, y, z);
        if (!voxel.Init())
          return false;

        // TODO(Cristian): Offset this by the chunk position offset.
        // TODO(Cristian): Later do a scene graph.
        voxel.set_position({x, y, z});
        SetCubeFace(*atlas_, &voxel, MinecraftCube::Face::kBack,
                    kGrassDirt, kTransparent);
        SetCubeFace(*atlas_, &voxel, MinecraftCube::Face::kFront,
                    kGrassDirt, kTransparent);
        SetCubeFace(*atlas_, &voxel, MinecraftCube::Face::kLeft,
                    kGrassDirt, kCrack4);
        SetCubeFace(*atlas_, &voxel, MinecraftCube::Face::kRight,
                    kGrassDirt, kTransparent);
        SetCubeFace(*atlas_, &voxel, MinecraftCube::Face::kTop,
                    kGrass, kCrack9);
        SetCubeFace(*atlas_, &voxel, MinecraftCube::Face::kBottom,
                    kDirt, kTransparent);
      }
    }
  }
  return true;
}

void VoxelChunk::Render(Shader* shader) {
  for (MinecraftCube& voxel : voxels_)
    voxel.Render(shader);
}

// VoxelTerrain ----------------------------------------------------------------

VoxelTerrain::VoxelTerrain(TextureAtlas* atlas) : atlas_(atlas) {}

bool VoxelTerrain::Init() {
  // Create the initial chunk.
  Pair3<int> coord = {0, 0, 0};
  VoxelChunk chunk(atlas_);
  if (!chunk.Init())
    return false;
  voxel_chunks_[std::move(coord)] = std::move(chunk);
  return true;
}

void VoxelTerrain::SetTextures(Shader* shader) const {
  atlas_->texture().Set(shader, GL_TEXTURE0);
  atlas_->texture().Set(shader, GL_TEXTURE1);
}

void VoxelTerrain::Render(Shader* shader) {
  SetTextures(shader);
  for (auto& [coord, voxel_chunk] : voxel_chunks_)
    voxel_chunk.Render(shader);
}

}  // namespace warhol
