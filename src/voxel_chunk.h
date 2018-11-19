// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

#include "src/math/vec.h"
#include "src/utils/clear_on_move.h"
#include "src/texture_atlas.h"
#include "src/utils/glm.h"
#include "src/utils/macros.h"

namespace warhol {

class Shader;

class Voxel {
 public:
  enum class Face {
    kFront,
    kBack,
    kLeft,
    kRight,
    kTop,
    kBottom,
  };

  Voxel();
  ~Voxel();
  DELETE_COPY_AND_ASSIGN(Voxel);
  DEFAULT_MOVE_AND_ASSIGN(Voxel);

  bool Init();

  // Set the textures index for the face. -1 means don't change the texture for
  // this layer.
  void SetFace(Face, int layer, Pair<Pair<float>> min_max_uvs);

  const glm::vec3& position() const { return position_; }
  void set_position(glm::vec3 pos) { position_ = pos; }

  void Render(Shader*);

 private:
  // TODO(Cristian): Voxels should not care about where they are. They only
  //                 know about their vertices.
  glm::vec3 position_;
  glm::mat4 model_ = glm::mat4(1.0f);

  ClearOnMove<uint32_t> vao_;
  ClearOnMove<uint32_t> vertex_vbo_;
  ClearOnMove<uint32_t> uv_vbo1_;
  ClearOnMove<uint32_t> uv_vbo2_;
  ClearOnMove<uint32_t> ebo_;

  std::vector<float> uvs1_;
  std::vector<float> uvs2_;
  bool initialized_ = false;
};

// How many voxels a voxel chunk is. Voxel chunks are assumed to be a cube.
constexpr size_t kVoxelChunkSize = 4;
constexpr size_t kVoxelChunkVoxelCount = kVoxelChunkSize *
                                         kVoxelChunkSize *
                                         kVoxelChunkSize;

struct VoxelElement {
  enum class Type : uint8_t {
    kNone = 0,
    kDirt = VoxelType::kDirt,
    kTopGrass = VoxelType::kGrass,

    kLast
  };

  Type type = Type::kDirt;

  explicit operator bool() const { return (uint8_t)type != 0; }
};

struct VoxelTypedChunk {
  Quad3<int> quad;
  VoxelElement::Type type = VoxelElement::Type::kNone;
};

// Represents a group of voxels in which the world is divided.
class VoxelChunk {
 public:
  VoxelChunk();
  VoxelChunk(TextureAtlas*);

  bool Init();
  bool InitialiazeGreedy();

  void Render(Shader*);
  // From the given voxel elements, a new mesh can be calculated that minimizes
  // the amount of vertices needed.
  void CalculateMesh();

  std::vector<std::vector<VoxelTypedChunk>> GreedyMesh();

  VoxelElement& GetVoxelElement(size_t x, size_t y, size_t z);

  // TODO(Cristian): Return a reference to the actual array?
  Voxel* voxels() { return voxels_; }
  size_t voxel_count() const { return ARRAY_SIZE(voxels_); }

  Voxel& GetVoxel(size_t x, size_t y, size_t z);

 private:
  VoxelElement elements_[kVoxelChunkVoxelCount];

  Voxel voxels_[kVoxelChunkVoxelCount];
  TextureAtlas* atlas_;   // Not owning. Must outlive.

  bool greedy = false;

  std::vector<std::vector<VoxelTypedChunk>> quads_;


  ClearOnMove<uint32_t> vao_;
  ClearOnMove<uint32_t> vbo_;
  ClearOnMove<uint32_t> ebo_;
};



}  // namespace warhol
