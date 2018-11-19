// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/voxel_terrain.h"

#include "src/shader.h"
#include "src/texture_atlas.h"

namespace warhol {

namespace {
}  // namespace

// VoxelTerrain ----------------------------------------------------------------

VoxelTerrain::VoxelTerrain(TextureAtlas* atlas) : atlas_(atlas) {}

bool VoxelTerrain::Init() {
  // Create the initial chunk.
  Pair3<int> coord = {0, 0, 0};
  VoxelChunk chunk(atlas_);
  if (!chunk.Init())
    return false;
  voxel_chunks_[std::move(coord)] = std::move(chunk);

  VoxelChunk chunk2(atlas_);
  chunk2.InitialiazeGreedy();
  voxel_chunks_[{1, 0, 1}] = std::move(chunk2);
  return true;
}

void VoxelTerrain::SetTextures(Shader* shader) const {
  atlas_->texture().Set(shader, GL_TEXTURE0);
  /* atlas_->texture().Set(shader, GL_TEXTURE1); */
}

void VoxelTerrain::Render(Shader* shader) {
  SetTextures(shader);
  for (auto& [coord, voxel_chunk] : voxel_chunks_)
    voxel_chunk.Render(shader);
}

}  // namespace warhol
