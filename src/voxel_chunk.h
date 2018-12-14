// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <array>
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
  // IMPORTANT: They *have* to be sequential, so no special indices can be used.
  enum class Type {
    kNone,
    kDirt,
    kGrassDirt,

    kCount,  // Not valid, should never be addressed.
  };
  static const char* TypeToString(Type);

  // Represents the texture coordinates a particular Voxel will face for each
  // particular face.
  struct FaceTexIndices {
    float x_min, x_max;
    float z_min, z_max;
    float y_min, y_max;
  };
  // Format is x-min/max, z-min/max, y-min/max.
  static const FaceTexIndices& GetFaceTexIndices(Type);

  explicit operator bool() const { return type != Type::kNone; }

  Type type = Type::kNone;
};

// Represents a face within the cube.
struct ExpandedVoxel {
  Quad3<int> quad;
  VoxelElement::Type type = VoxelElement::Type::kNone;
};

struct TypedFace {
  static constexpr int kVertCount = 4 * 3;
  static constexpr int kUVCount = 4 * 2;
  float verts[kVertCount];
  float uvs[kUVCount];
  float tex_index;

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
  VoxelElement& GetVoxelElement(int index);

  // TODO(Cristian): Return a reference to the actual array?
  /* Voxel* voxels() { return voxels_; } */
  /* size_t voxel_count() const { return ARRAY_SIZE(voxels_); } */
  /* Voxel& GetVoxel(size_t x, size_t y, size_t z); */

  uint32_t vao() const { return vao_.value; }
  uint32_t vbo() const { return vbo_.value; }
  uint32_t ebo() const { return ebo_.value; }

  const std::vector<bool>& mask() const { return mask_; }

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
  std::vector<bool> mask_;

  std::vector<TypedFace> faces_;
  size_t face_count_;

  TextureArray2D* tex_array_;   // Not owning.

  // TODO(Cristian): Stop leaking these.
  ClearOnMove<uint32_t> vao_;
  ClearOnMove<uint32_t> vbo_;
  ClearOnMove<uint32_t> ebo_;
};

}  // namespace warhol
