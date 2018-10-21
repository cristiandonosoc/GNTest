// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/model/minecraft_cube.h"

#include <GL/gl3w.h>

#include "src/shader.h"
#include "src/texture_atlas.h"
#include "src/utils/glm_impl.h"

namespace warhol {

namespace {

float indexed_vertices[] = {
  // Front.
     0.5f,  -0.5f,  -0.5f,
     0.5f,  -0.5f, 0.5f,
     0.5f, 0.5f, 0.5f,
     0.5f, 0.5f,  -0.5f,

     // Back.
     -0.5f,  -0.5f,  -0.5f,
     -0.5f,  -0.5f, 0.5f,
     -0.5f, 0.5f, 0.5f,
     -0.5f, 0.5f,  -0.5f,

       // Left.
  -0.5f,  -0.5f, -0.5f,
  0.5f, -0.5f, -0.5f,
  0.5f, 0.5f, -0.5f,
  -0.5f,  0.5f, -0.5f,

     // Right.
-0.5f, -0.5f, 0.5f,
0.5f, -0.5f, 0.5f,
0.5f, 0.5f, 0.5f,
-0.5f, 0.5f, 0.5f,

// Top.
-0.5f, 0.5f, -0.5f,
 0.5f, 0.5f, -0.5f,
0.5f,  0.5f, 0.5f,
-0.5f, 0.5f,  0.5f,

// Bottom.
-0.5f, -0.5f, -0.5f,
 0.5f, -0.5f, -0.5f,
0.5f,  -0.5f, 0.5f,
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

MinecraftCube::MinecraftCube(TextureAtlas* atlas) : atlas_(atlas) {}

bool MinecraftCube::Init() {
  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  uint32_t buffers[4];
  glGenBuffers(3, buffers);
  vertex_vbo_ = buffers[0];
  uv_vbo1_ = buffers[1];
  uv_vbo2_ = buffers[2];
  ebo_ = buffers[3];

  uvs1_.reserve(ARRAY_SIZE(indexed_uvs));
  uvs2_.reserve(ARRAY_SIZE(indexed_uvs));
  for (size_t i = 0; i < ARRAY_SIZE(indexed_uvs); i++) {
    uvs1_.emplace_back(indexed_uvs[i]);
    uvs2_.emplace_back(indexed_uvs[i]);
  }

  // Vertices.
  glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo_);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(indexed_vertices),
               indexed_vertices,
               GL_STATIC_DRAW);
  // How to interpret the buffer.
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // UV
  glBindBuffer(GL_ARRAY_BUFFER, uv_vbo1_);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * uvs1_.size(),
               uvs1_.data(),
               GL_DYNAMIC_DRAW);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);

  // UV2
  glBindBuffer(GL_ARRAY_BUFFER, uv_vbo2_);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * uvs2_.size(),
               uvs2_.data(),
               GL_DYNAMIC_DRAW);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(2);

  // Indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glBindVertexArray(NULL);
  glBindBuffer(GL_ARRAY_BUFFER, NULL);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);

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

void MinecraftCube::SetTextures(Shader* shader) const {
  atlas_->texture().Set(shader, GL_TEXTURE0);
  atlas_->texture().Set(shader, GL_TEXTURE1);
}


namespace {

void
ChangeUV(const TextureAtlas& atlas,
         MinecraftCube::Face face,
         int vbo,
         int index,
         std::vector<float>* uvs) {
  auto uv_coords = atlas.GetUVs(index);
  uint32_t offset = 2 * 4 * (uint32_t)face - (uint32_t)MinecraftCube::Face::kFront;
  uvs->at(offset + 0) = uv_coords.bottom_left.x;
  uvs->at(offset + 1) = uv_coords.bottom_left.y;
  uvs->at(offset + 2) = uv_coords.top_right.x;
  uvs->at(offset + 3) = uv_coords.bottom_left.y;
  uvs->at(offset + 4) = uv_coords.top_right.x;
  uvs->at(offset + 5) = uv_coords.top_right.y;
  uvs->at(offset + 6) = uv_coords.bottom_left.x;
  uvs->at(offset + 7) = uv_coords.top_right.y;

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferSubData(GL_ARRAY_BUFFER,
                  offset * sizeof(float),
                  8 * sizeof(float),
                  uvs->data() + offset);
  glBindBuffer(GL_ARRAY_BUFFER, NULL);
}

}  // namespace


void MinecraftCube::SetFace(MinecraftCube::Face face, int index1, int index2) {
  if (index1 >= 0)
    ChangeUV(*atlas_, face, uv_vbo1_, index1, &uvs1_);
  if (index2 >= 0)
    ChangeUV(*atlas_, face, uv_vbo2_, index2, &uvs2_);
}


void MinecraftCube::Render(Shader* shader) {
  glBindVertexArray(vao_);
  // TODO(donosoc): Do this only when needed.
  model_ = glm::translate(glm::mat4(1.0f), position_);
  shader->SetMat4("model", model_);

  atlas_->texture().Set(shader, GL_TEXTURE0);

  /* glDrawArrays(GL_TRIANGLES, 0, 36); */
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);

}

}  // namespace warhol
