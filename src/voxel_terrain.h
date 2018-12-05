// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <unordered_map>

#include "src/math/vec.h"
#include "src/voxel_chunk.h"
#include "src/texture_array.h"

namespace warhol {

class Shader;
class TextureAtlas;

class VoxelTerrain {
 public:
  // TODO(Cristian): Use my version of a hash table.
  using VoxelChunkHash = std::unordered_map<Pair3<int>, VoxelChunk,
                                            HashPair3<int>>;

  VoxelTerrain(TextureArray2D*);
  bool Init();


  void Render(Shader*);

  VoxelChunkHash& voxel_chunks() { return voxel_chunks_; }

 private:
  VoxelChunkHash voxel_chunks_;

  TextureArray2D* tex_array_;   // Not owning. Must outlive.
};

}  // namespace warhol
