// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <unordered_map>

#include "src/math/vec.h"
/* #include "src/model/minecraft_cube.h" */

namespace warhol {

class MinecraftCube;
class Shader;
class TextureAtlas;

class VoxelTerrain {
 public:
  // TODO(Cristian): Use my version of a hash table.
  using TerrainHash = std::unordered_map<Pair<int>, MinecraftCube,
                                         HashPair<int>>;

  VoxelTerrain(TextureAtlas*);

  // Will load the atlas as the current texture.
  void SetTextures(Shader* shader) const;

  void Render();

  bool Init();

 private:
  TerrainHash terrain_;
  TextureAtlas* atlas_;   // Not owning. Must outlive.
};

}  // namespace warhol
