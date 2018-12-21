// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <array>
#include <vector>

#include "src/atlas_data.h"
#include "src/math/vec.h"
#include "src/mesh.h"
#include "src/utils/clear_on_move.h"
#include "src/utils/glm.h"
#include "src/utils/macros.h"

namespace warhol {

class Shader;
class TextureArray2D;
class TextureAtlas;

// How many voxels a voxel chunk is. Voxel chunks are assumed to be a cube.
constexpr size_t kVoxelChunkSize = 8;
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
  Vec3 face_color = {};

  VoxelType type = VoxelType::kNone;
  // TODO(Cristian): Normals.
};

// Represents a group of voxels in which the world is divided.
class VoxelChunk {
 public:
  VoxelChunk();

  bool Init();
  void CalculateMesh();
  void Render(Shader*, Vec3 offset);

  VoxelElement& operator[](int index);
  VoxelElement& GetVoxel(const Pair3<int>&);
  VoxelElement& GetVoxel(int x, int y, int z);
  VoxelElement& GetVoxel(int index);

  const std::vector<bool>& mask() const { return mask_; }

  bool initialized() const { return initialized_; }

 private:
  std::vector<ExpandedVoxel> ExpandVoxels();
  std::vector<TypedFace> CalculateFaces();

  VoxelElement elements_[kVoxelChunkVoxelCount];

  // TODO(Cristian): This information is not necessary to be kept track of.
  //                 Can be calculated on the fly.
  std::vector<TypedFace> faces_;
  std::vector<bool> mask_;

  Mesh mesh_;

  bool initialized_ = false;
};

}  // namespace warhol
