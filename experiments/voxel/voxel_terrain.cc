// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "experiments/voxel/voxel_terrain.h"

#include <warhol/debug/volumes.h>
#include <warhol/multithreading/work_queue.h>
#include <warhol/shader.h>
#include <warhol/texture_atlas.h>
#include <warhol/utils/log.h>

#include <thread>

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
  temp_metadata_[metadata.chunk_coord] = std::move(metadata);
}

void VoxelTerrain::Update() {
  for (auto& [coord, metadata] : temp_metadata_) {
    auto it = voxel_chunks_.find(coord);
    assert(it != voxel_chunks_.end());
    VoxelChunk& chunk = it->second;
    /* if (!chunk.initialized()) */
    /*   chunk.Init(); */
    chunk.CalculateMesh();
  }

  temp_metadata_.clear();
}

namespace {

struct WorkData {
  VoxelChunk* chunk;
  int id;
};

void CalculateMeshInWorker(void* data) {
  WorkData* wd = (WorkData*)data;
  /* LOG(DEBUG) << "Chunk " << wd->id << " done by thread " << std::this_thread::get_id(); */
  wd->chunk->CalculateMesh();
}

}  // namespace

void VoxelTerrain::UpdateMT(WorkQueue* queue) {
  (void)queue;
  static WorkData work_data[1024];
  int i = 0;
  for (auto& [coord, metadata] : temp_metadata_) {
    auto it = voxel_chunks_.find(metadata.chunk_coord);
    assert(it != voxel_chunks_.end());
    VoxelChunk& chunk = it->second;
    /* if (!chunk.initialized()) */
    /*   chunk.Init(); */

    /* LOG(DEBUG) << "Pushing chunk: " << metadata.chunk_coord.ToString(); */


    work_data[i] = {&chunk, i};

    PushTask(queue, {CalculateMeshInWorker, work_data + i});
    i++;

    /* chunk.CalculateMesh(); */
  }

  LOG(DEBUG) << "Pushed " << i << " chunks";

  temp_metadata_.clear();
}

void VoxelTerrain::Render(Shader* shader, bool debug) {
  GL_CALL(glActiveTexture, GL_TEXTURE0);
  GL_CALL(glBindTexture, GL_TEXTURE_2D_ARRAY, tex_array_->handle());

  auto [unit_index, unit_name] = TextureUnitToUniform(GL_TEXTURE0);
  shader->SetInt(unit_name, unit_index);

      static bool a = true;
  for (auto& [coord, voxel_chunk] : voxel_chunks_) {
    Vec3 fcoord{coord.x, coord.y, coord.z};
    fcoord *= kVoxelChunkSize;
    voxel_chunk.Render(shader, fcoord);

    if (debug) {
      auto c = coord;
      c *= (int)kVoxelChunkSize;
      DrawChunkVolume(c);
    }
  }
  a = false;
}


void VoxelTerrain::DrawChunkVolume(Pair3<int> coord, Vec3 color) {
  float s = kVoxelChunkSize / 2;
  Vec3 c = {coord.x, coord.y, coord.z};
  Vec3 size = {s, s, s};
  c += size;
  DebugVolumes::AABB(c, size, color);
}

}  // namespace warhol
