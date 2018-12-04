// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

#include "src/atlas_data.h"
#include "src/math/vec.h"
#include "src/utils/clear_on_move.h"
#include "src/utils/glm.h"
#include "src/utils/macros.h"

namespace warhol {

class Shader;
class TextureArray2D;
class TextureAtlas;

// How many voxels a voxel chunk is. Voxel chunks are assumed to be a cube.
constexpr size_t kVoxelChunkSize = 4;
constexpr size_t kVoxelChunkVoxelCount = kVoxelChunkSize *
                                         kVoxelChunkSize *
                                         kVoxelChunkSize;

struct VoxelElement {
  VoxelType type = VoxelType::kNone;

  explicit operator bool() const { return type != VoxelType::kNone; }
};

// Represents a face within the cube.
struct ExpandedVoxel {
  Quad3<int> quad;
  VoxelType type = VoxelType::kDirt;
};

struct TypedFace {
  static constexpr int kVertCount = 4 * 3;
  static constexpr int kUVCount = 4 * 2;
  float verts[kVertCount];
  float uvs[kUVCount];
  uint32_t tex_index;

  VoxelType type = VoxelType::kNone;
  // TODO(Cristian): Normals.
};

// Represents a group of voxels in which the world is divided.
class VoxelChunk {
 public:
  VoxelChunk();
  VoxelChunk(TextureArray2D*);

  bool Init();
  void CalculateMesh();
  void Render(Shader*);

  VoxelElement& operator[](int index);
  VoxelElement& GetVoxelElement(int x, int y, int z);

  // TODO(Cristian): Return a reference to the actual array?
  /* Voxel* voxels() { return voxels_; } */
  /* size_t voxel_count() const { return ARRAY_SIZE(voxels_); } */
  /* Voxel& GetVoxel(size_t x, size_t y, size_t z); */

  uint32_t vao() const { return vao_.value; }
  uint32_t vbo() const { return vbo_.value; }
  uint32_t ebo() const { return ebo_.value; }

 private:
  std::vector<TypedFace> CalculateFaces();

  std::vector<ExpandedVoxel> ExpandVoxels();
  std::vector<TypedFace>
  CalculateFacesX(VoxelType, int z, int z_to_check, Quad3<int>);
  std::vector<TypedFace>
  CalculateFacesY(VoxelType, int z, int z_to_check, Quad3<int>);
  std::vector<TypedFace>
  CalculateFacesZ(VoxelType, int z, int z_to_check, Quad3<int>);

  VoxelElement elements_[kVoxelChunkVoxelCount];
  std::vector<TypedFace> faces_;
  size_t face_count_;

  TextureArray2D* tex_array_;   // Not owning.

  // TODO(Cristian): Stop leaking these.
  ClearOnMove<uint32_t> vao_;
  ClearOnMove<uint32_t> vbo_;
  ClearOnMove<uint32_t> ebo_;
};

}  // namespace warhol
