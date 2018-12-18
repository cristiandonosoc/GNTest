// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/voxel_terrain.h"

#include "src/shader.h"
#include "src/texture_atlas.h"
#include "src/utils/log.h"

namespace warhol {

namespace {
}  // namespace

// VoxelTerrain ----------------------------------------------------------------

VoxelTerrain::VoxelTerrain(TextureArray2D* tex_array) : tex_array_(tex_array) {}

bool VoxelTerrain::Init() {
  // Create the initial chunk.
  Pair3<int> coord = {0, 0, 0};
  VoxelChunk chunk;
  if (!chunk.Init())
    return false;
  chunk.CalculateMesh();
  voxel_chunks_[std::move(coord)] = std::move(chunk);

  /* VoxelChunk chunk2(atlas_); */
  /* chunk2.Init(); */
  /* chunk2.CalculateMesh(); */
  /* voxel_chunks_[{1, 0, 1}] = std::move(chunk2); */
  return true;
}

void VoxelTerrain::Render(Shader* shader) {
  GL_CALL(glActiveTexture, GL_TEXTURE0);
  GL_CALL(glBindTexture, GL_TEXTURE_2D_ARRAY, tex_array_->handle());

  auto [unit_index, unit_name] = TextureUnitToUniform(GL_TEXTURE0);
  shader->SetInt(unit_name, unit_index);

  for (auto& [coord, voxel_chunk] : voxel_chunks_)
    voxel_chunk.Render(shader);
}

}  // namespace warhol
