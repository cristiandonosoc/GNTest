// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <unordered_map>

#include "src/math/vec.h"
#include "src/voxel_chunk.h"

namespace warhol {

class Shader;
class TextureAtlas;

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

  VoxelChunkHash& voxel_chunks() { return voxel_chunks_; }

 private:
  VoxelChunkHash voxel_chunks_;

  TextureAtlas* atlas_;   // Not owning. Must outlive.
};

}  // namespace warhol
