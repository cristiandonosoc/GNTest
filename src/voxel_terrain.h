// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <unordered_map>

#include "src/math/vec.h"
#include "src/model/minecraft_cube.h"

namespace warhol {

class Shader;
class TextureAtlas;

// How many voxels a voxel chunk is. Voxel chunks are assumed to be a cube.
constexpr size_t kVoxelChunkSize = 4;
constexpr size_t kVoxelChunkVoxelCount = kVoxelChunkSize *
                                         kVoxelChunkSize *
                                         kVoxelChunkSize;

// Represents a group of voxels in which the world is divided.
class VoxelChunk {
 public:
  VoxelChunk();
  VoxelChunk(TextureAtlas*);
  bool Init();
  void Render(Shader*);

  // TODO(Cristian): Return a reference to the actual array?
  MinecraftCube* voxels() { return voxels_; }
  size_t voxel_count() const { return ARRAY_SIZE(voxels_); }

  MinecraftCube& GetVoxel(size_t x, size_t y, size_t z);

 private:
  MinecraftCube voxels_[kVoxelChunkVoxelCount];
  TextureAtlas* atlas_;   // Not owning. Must outlive.
};

class VoxelTerrain {
 public:
  // TODO(Cristian): Use my version of a hash table.
  using VoxelChunkHash = std::unordered_map<Pair3<int>, VoxelChunk,
                                            HashPair3<int>>;

  VoxelTerrain(TextureAtlas*);

  // Will load the atlas as the current texture.
  void SetTextures(Shader* shader) const;

  void Render(Shader*);

  bool Init();

 private:
  VoxelChunkHash voxel_chunks_;

  TextureAtlas* atlas_;   // Not owning. Must outlive.
};

}  // namespace warhol
