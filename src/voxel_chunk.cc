// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/voxel_chunk.h"

#include <bitset>


#include "src/graphics/GL/utils.h"
#include "src/shader.h"
#include "src/texture_atlas.h"
#include "src/utils/coords.h"
#include "src/utils/glm_impl.h"

namespace warhol {

namespace {

float indexed_vertices[] = {
  // Front.
  0.5f,  -0.5f, -0.5f,
  0.5f,  -0.5f,  0.5f,
  0.5f,   0.5f,  0.5f,
  0.5f,   0.5f, -0.5f,

  // Back.
  -0.5f, -0.5f, -0.5f,
  -0.5f, -0.5f,  0.5f,
  -0.5f,  0.5f,  0.5f,
  -0.5f,  0.5f, -0.5f,

  // Left.
  -0.5f, -0.5f, -0.5f,
   0.5f, -0.5f, -0.5f,
   0.5f,  0.5f, -0.5f,
  -0.5f,  0.5f, -0.5f,

  // Right.
  -0.5f, -0.5f, 0.5f,
   0.5f, -0.5f, 0.5f,
   0.5f,  0.5f, 0.5f,
  -0.5f,  0.5f, 0.5f,

  // Top.
  -0.5f, 0.5f, -0.5f,
   0.5f, 0.5f, -0.5f,
   0.5f, 0.5f,  0.5f,
  -0.5f, 0.5f,  0.5f,

  // Bottom.
  -0.5f, -0.5f, -0.5f,
   0.5f, -0.5f, -0.5f,
   0.5f, -0.5f,  0.5f,
  -0.5f, -0.5f,  0.5f,
};

float indexed_uvs[] = {
  0.0f, 0.0f,
  1.0f, 0.0f,
  1.0f, 1.0f,
  0.0f, 1.0f,

  0.0f, 0.0f,
  1.0f, 0.0f,
  1.0f, 1.0f,
  0.0f, 1.0f,

  0.0f, 0.0f,
  1.0f, 0.0f,
  1.0f, 1.0f,
  0.0f, 1.0f,

  0.0f, 0.0f,
  1.0f, 0.0f,
  1.0f, 1.0f,
  0.0f, 1.0f,

  0.0f, 0.0f,
  1.0f, 0.0f,
  1.0f, 1.0f,
  0.0f, 1.0f,

  0.0f, 0.0f,
  1.0f, 0.0f,
  1.0f, 1.0f,
  0.0f, 1.0f,
};

uint32_t indices[] = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
    8, 9, 10, 10, 11, 8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20,
};

}  // namespace

Voxel::Voxel() = default;
Voxel::~Voxel() {
  if (vao_.value) {
    glDeleteVertexArrays(1, &vao_.value);
    vao_.clear();
  }
  if (vertex_vbo_.value) {
    glDeleteBuffers(1, &vertex_vbo_.value);
    vao_.clear();
  }
  if (uv_vbo1_.value) {
    glDeleteBuffers(1, &uv_vbo1_.value);
    uv_vbo1_.clear();
  }
  if (uv_vbo2_.value) {
    glDeleteBuffers(1, &uv_vbo2_.value);
    uv_vbo1_.clear();
  }
  if (ebo_.value) {
    glDeleteBuffers(1, &ebo_.value);
    ebo_.clear();
  }
}


bool Voxel::Init() {
  glGenVertexArrays(1, &vao_.value);
  glBindVertexArray(vao_.value);

  uint32_t buffers[4];
  glGenBuffers(ARRAY_SIZE(buffers), buffers);
  vertex_vbo_ = buffers[0];
  uv_vbo1_ = buffers[1];
  uv_vbo2_ = buffers[2];
  ebo_ = buffers[3];

  if (CHECK_GL_ERRORS("Creating buffers"))
    exit(1);

  uvs1_.reserve(ARRAY_SIZE(indexed_uvs));
  uvs2_.reserve(ARRAY_SIZE(indexed_uvs));
  for (size_t i = 0; i < ARRAY_SIZE(indexed_uvs); i++) {
    uvs1_.emplace_back(indexed_uvs[i]);
    uvs2_.emplace_back(indexed_uvs[i]);
  }

  // Vertices.
  glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo_.value);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(indexed_vertices),
               indexed_vertices,
               GL_STATIC_DRAW);
  // How to interpret the buffer.
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  if (CHECK_GL_ERRORS("Buffering vertices"))
    exit(1);

  // UV
  glBindBuffer(GL_ARRAY_BUFFER, uv_vbo1_.value);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * uvs1_.size(),
               uvs1_.data(),
               GL_DYNAMIC_DRAW);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);

  if (CHECK_GL_ERRORS("Buffering UV1"))
    exit(1);

  // UV2
  glBindBuffer(GL_ARRAY_BUFFER, uv_vbo2_.value);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * uvs2_.size(),
               uvs2_.data(),
               GL_DYNAMIC_DRAW);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(2);

  if (CHECK_GL_ERRORS("Buffering UV2"))
    exit(1);

  // Indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_.value);
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  if (CHECK_GL_ERRORS("Buffering indices"))
    exit(1);

  glBindVertexArray(NULL);
  glBindBuffer(GL_ARRAY_BUFFER, NULL);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);

  if (CHECK_GL_ERRORS("Unbinding"))
    exit(1);

  initialized_ = true;
  return true;
}

#include <sstream>

void PrintUVs(const std::vector<float>& uvs) {
  std::stringstream ss;
  ss << "UVS: " << std::endl;
  for (size_t i = 0; i < 6; i++) {
    size_t offset = 8 * i;
    ss
    << uvs[offset + 0] << ", " << uvs[offset + 1] << std::endl
    << uvs[offset + 2] << ", " << uvs[offset + 3] << std::endl
    << uvs[offset + 4] << ", " << uvs[offset + 5] << std::endl
    << uvs[offset + 6] << ", " << uvs[offset + 7] << std::endl
    << "---------------------------------------" << std::endl;
  }
  LOG(DEBUG) << ss.str();
}

namespace {

void
ChangeUV(Voxel::Face face,
         Pair<Pair<float>> min_max_uvs,
         int vbo,
         std::vector<float>* uvs) {
  /* auto uv_coords = atlas.GetUVs(index); */
  uint32_t offset =
      2 * 4 * (uint32_t)face - (uint32_t)Voxel::Face::kFront;
  uvs->at(offset + 0) = min_max_uvs.x.x;
  uvs->at(offset + 1) = min_max_uvs.x.y;
  uvs->at(offset + 2) = min_max_uvs.y.x;
  uvs->at(offset + 3) = min_max_uvs.x.y;
  uvs->at(offset + 4) = min_max_uvs.y.x;
  uvs->at(offset + 5) = min_max_uvs.y.y;
  uvs->at(offset + 6) = min_max_uvs.x.x;
  uvs->at(offset + 7) = min_max_uvs.y.y;

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferSubData(GL_ARRAY_BUFFER,
                  offset * sizeof(float),
                  8 * sizeof(float),
                  uvs->data() + offset);
  glBindBuffer(GL_ARRAY_BUFFER, NULL);
}

#if 0

// TODO(Cristian): Extend this to separate dimensions.
void GreedyMesh(size_t side) {

  for (size_t dim = 0; dim < 3; dim++) {
    size_t dim_up = (dim + 1) % 3;
    size_t dim_down = (dim + 2) % 3;

    Pair3<size_t> x = {};
    Pair3<size_t> quad = {};
    quad[dim] = 1;
    std::bitset<kVoxelChunkSize * kVoxelChunkSize> mask;

    for (x[dim] = -1; x[dim] < side;) {
      // Compute mask. //TODO(Cristian): What does this mask mean?
      size_t n = 0;
      for (x[dim_down] = 0; x[dim_down] < side; x[dim_down]++) {
        for (x[dim_up] = 0; x [dim_up] < side; x[dim_up]++) {

        }
      }


    }
  }


}


#endif

}  // namespace




void
Voxel::SetFace(Voxel::Face face,
                       int layer,
                       Pair<Pair<float>> min_max_uvs) {
  if (layer == 0) {
    ChangeUV(face, std::move(min_max_uvs), uv_vbo1_.value, &uvs1_);
  } else if (layer == 1) {
    ChangeUV(face, std::move(min_max_uvs), uv_vbo2_.value, &uvs2_);
  } else {
    LOG(ERROR) << "Wrong layer count: " << layer;
    exit(1);
  }
}

void Voxel::Render(Shader* shader) {
  assert(initialized_);
  glBindVertexArray(vao_.value);
  // TODO(donosoc): Do this only when needed.
  model_ = glm::translate(glm::mat4(1.0f), position_);
  shader->SetMat4("model", model_);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);
}

}  // namespace warhol
