// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/voxel_terrain.h"

#include "src/shader.h"
#include "src/texture_atlas.h"
#include "src/utils/log.h"

namespace warhol {

namespace {

struct TieredCoord {
  Pair3<int> chunk_coord;
  Pair3<int> internal_coord;
};

TieredCoord GlobalToTiered(Pair3<int> coord) {
  TieredCoord result = {};
  result.chunk_coord.x = coord.x / kVoxelChunkSize;
  result.chunk_coord.y = coord.y / kVoxelChunkSize;
  result.chunk_coord.z = coord.z / kVoxelChunkSize;

  result.internal_coord.x = coord.x % kVoxelChunkSize;
  result.internal_coord.y = coord.y % kVoxelChunkSize;
  result.internal_coord.z = coord.z % kVoxelChunkSize;

  return result;
}

}  // namespace

// VoxelTerrain ----------------------------------------------------------------

VoxelTerrain::VoxelTerrain(TextureArray2D* tex_array) : tex_array_(tex_array) {}

bool VoxelTerrain::Init() {
  initialized_ = true;
  return true;
}

void VoxelTerrain::SetVoxel(Pair3<int> coord, VoxelElement::Type type) {
  TieredCoord tiered_coord = GlobalToTiered(coord);

  auto& chunk = voxel_chunks_[tiered_coord.chunk_coord];
  chunk.GetVoxel(tiered_coord.internal_coord).type = type;

  VoxelChunkMetadata metadata = {};
  metadata.chunk_coord = tiered_coord.chunk_coord;
  metadata.dirty = true;
  temp_metadata_.push_back(std::move(metadata));
}

void VoxelTerrain::Update() {
  for (auto& metadata : temp_metadata_) {
    auto it = voxel_chunks_.find(metadata.chunk_coord);
    assert(it != voxel_chunks_.end());
    VoxelChunk& chunk = it->second;
    if (!chunk.initialized())
      chunk.Init();
    chunk.CalculateMesh();
  }

  temp_metadata_.clear();
}

void VoxelTerrain::Render(Shader* shader) {
  GL_CALL(glActiveTexture, GL_TEXTURE0);
  GL_CALL(glBindTexture, GL_TEXTURE_2D_ARRAY, tex_array_->handle());

  auto [unit_index, unit_name] = TextureUnitToUniform(GL_TEXTURE0);
  shader->SetInt(unit_name, unit_index);

  for (auto& [coord, voxel_chunk] : voxel_chunks_) {
    Vec3 fcoord{coord.x, coord.y, coord.z};
    fcoord *= kVoxelChunkSize;
    voxel_chunk.Render(shader, fcoord);
  }
}

}  // namespace warhol
