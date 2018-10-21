// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/model/minecraft_cube.h"

#include <GL/gl3w.h>

#include "src/shader.h"
#include "src/texture_atlas.h"
#include "src/utils/glm_impl.h"

namespace warhol {

namespace {

/* float vertices[] = { */
/*     -0.5f, -0.5f, -0.5f, */
/*      0.5f, -0.5f, -0.5f, */
/*      0.5f,  0.5f, -0.5f, */
/*      0.5f,  0.5f, -0.5f, */
/*     -0.5f,  0.5f, -0.5f, */
/*     -0.5f, -0.5f, -0.5f, */

/*     -0.5f, -0.5f,  0.5f, */
/*      0.5f, -0.5f,  0.5f, */
/*      0.5f,  0.5f,  0.5f, */
/*      0.5f,  0.5f,  0.5f, */
/*     -0.5f,  0.5f,  0.5f, */
/*     -0.5f, -0.5f,  0.5f, */

/*     -0.5f,  0.5f,  0.5f, */
/*     -0.5f,  0.5f, -0.5f, */
/*     -0.5f, -0.5f, -0.5f, */
/*     -0.5f, -0.5f, -0.5f, */
/*     -0.5f, -0.5f,  0.5f, */
/*     -0.5f,  0.5f,  0.5f, */

/*      0.5f,  0.5f,  0.5f, */
/*      0.5f,  0.5f, -0.5f, */
/*      0.5f, -0.5f, -0.5f, */
/*      0.5f, -0.5f, -0.5f, */
/*      0.5f, -0.5f,  0.5f, */
/*      0.5f,  0.5f,  0.5f, */

/*     -0.5f, -0.5f, -0.5f, */
/*      0.5f, -0.5f, -0.5f, */
/*      0.5f, -0.5f,  0.5f, */
/*      0.5f, -0.5f,  0.5f, */
/*     -0.5f, -0.5f,  0.5f, */
/*     -0.5f, -0.5f, -0.5f, */

/*     -0.5f,  0.5f, -0.5f, */
/*      0.5f,  0.5f, -0.5f, */
/*      0.5f,  0.5f,  0.5f, */
/*      0.5f,  0.5f,  0.5f, */
/*     -0.5f,  0.5f,  0.5f, */
/*     -0.5f,  0.5f, -0.5f, */
/* }; */

float indexed_vertices[] = {
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
//     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
//    -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
//     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
//    -0.5f, -0.5f,  0.5f,

    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
//    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
//    -0.5f,  0.5f,  0.5f,

     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
//     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
//     0.5f,  0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
//     0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
//    -0.5f, -0.5f, -0.5f,

    -0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,
//     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
//    -0.5f,  0.5f, -0.5f,
};

/* float uvs[] = { */
/*     0.0f, 0.0f, */
/*     1.0f, 0.0f, */
/*     1.0f, 1.0f, */
/*     1.0f, 1.0f, */
/*     0.0f, 1.0f, */
/*     0.0f, 0.0f, */

/*     0.0f, 0.0f, */
/*     1.0f, 0.0f, */
/*     1.0f, 1.0f, */
/*     1.0f, 1.0f, */
/*     0.0f, 1.0f, */
/*     0.0f, 0.0f, */

/*     1.0f, 0.0f, */
/*     1.0f, 1.0f, */
/*     0.0f, 1.0f, */
/*     0.0f, 1.0f, */
/*     0.0f, 0.0f, */
/*     1.0f, 0.0f, */

/*     1.0f, 0.0f, */
/*     1.0f, 1.0f, */
/*     0.0f, 1.0f, */
/*     0.0f, 1.0f, */
/*     0.0f, 0.0f, */
/*     1.0f, 0.0f, */

/*     0.0f, 1.0f, */
/*     1.0f, 1.0f, */
/*     1.0f, 0.0f, */
/*     1.0f, 0.0f, */
/*     0.0f, 0.0f, */
/*     0.0f, 1.0f, */

/*     0.0f, 1.0f, */
/*     1.0f, 1.0f, */
/*     1.0f, 0.0f, */
/*     1.0f, 0.0f, */
/*     0.0f, 0.0f, */
/*     0.0f, 1.0f */
/* }; */

float indexed_uvs[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    // 1.0f, 1.0f,
    0.0f, 1.0f,
    // 0.0f, 0.0f,

    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    // 1.0f, 1.0f,
    0.0f, 1.0f,
    // 0.0f, 0.0f,

    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    // 0.0f, 1.0f,
    0.0f, 0.0f,
    // 1.0f, 0.0f,

    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    // 0.0f, 1.0f,
    0.0f, 0.0f,
    // 1.0f, 0.0f,

    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    // 1.0f, 0.0f,
    0.0f, 0.0f,
    // 0.0f, 1.0f,

    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    // 1.0f, 0.0f,
    0.0f, 0.0f,
    // 0.0f, 1.0f
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

  uint32_t buffers[3];
  glGenBuffers(3, buffers);
  vertex_vbo_ = buffers[0];
  uv_vbo_ = buffers[1];
  ebo_ = buffers[2];

  uvs_.reserve(ARRAY_SIZE(indexed_uvs));
  for (size_t i = 0; i < ARRAY_SIZE(indexed_uvs); i++)
    uvs_.emplace_back(indexed_uvs[i]);


  // Vertices.
  glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo_);
  /* glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); */
  glBufferData(GL_ARRAY_BUFFER, sizeof(indexed_vertices), indexed_vertices, GL_STATIC_DRAW);
  // How to interpret the buffer.
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // UVs.
  glBindBuffer(GL_ARRAY_BUFFER, uv_vbo_);
  /* glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_DYNAMIC_DRAW); */
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * uvs_.size(), uvs_.data(), GL_DYNAMIC_DRAW);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(2);

  // Indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

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

void MinecraftCube::SetFace(MinecraftCube::Face face, size_t index) {
  LOG(DEBUG) << "BEFORE";
  PrintUVs(uvs_);
  auto uvs = atlas_->GetUVs(index);
  uint32_t offset = 2 * 4 * (uint32_t)face - (uint32_t)MinecraftCube::Face::kFront;
  uvs_[offset + 0] = uvs.bottom_left.x;
  uvs_[offset + 1] = uvs.bottom_left.y;
  uvs_[offset + 2] = uvs.top_right.x;
  uvs_[offset + 3] = uvs.bottom_left.y;
  uvs_[offset + 4] = uvs.top_right.x;
  uvs_[offset + 5] = uvs.top_right.y;
  uvs_[offset + 6] = uvs.bottom_left.x;
  uvs_[offset + 7] = uvs.top_right.y;

  glBindBuffer(GL_ARRAY_BUFFER, uv_vbo_);
  glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(float), 8 * sizeof(float), uvs_.data() + offset);
  glBindBuffer(GL_ARRAY_BUFFER, NULL);


  LOG(DEBUG) << "AFTER";
  PrintUVs(uvs_);

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
