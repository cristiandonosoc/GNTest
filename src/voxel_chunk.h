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
  // TODO(Cristian): Normals.
};

// Represents a group of voxels in which the world is divided.
class VoxelChunk {
 public:
  VoxelChunk();
  VoxelChunk(TextureAtlas*);

  bool Init();
  bool InitialiazeGreedy();

  void Render(Shader*);

  void CalculateGreedyMesh();



  // From the given voxel elements, a new mesh can be calculated that minimizes
  // the amount of vertices needed.
  void CalculateMesh();

  VoxelElement& operator[](int index);
  VoxelElement& GetVoxelElement(int x, int y, int z);

  // TODO(Cristian): Return a reference to the actual array?
  Voxel* voxels() { return voxels_; }
  size_t voxel_count() const { return ARRAY_SIZE(voxels_); }

  Voxel& GetVoxel(size_t x, size_t y, size_t z);

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

  Voxel voxels_[kVoxelChunkVoxelCount];
  TextureAtlas* atlas_;   // Not owning. Must outlive.

  bool greedy = false;

  /* std::vector<std::vector<VoxelTypeQuad>> quads_; */


  ClearOnMove<uint32_t> vao_;
  ClearOnMove<uint32_t> vbo_;
  ClearOnMove<uint32_t> ebo_;
};



}  // namespace warhol
