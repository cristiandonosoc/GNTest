// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

#include "src/math/vec.h"
#include "src/utils/clear_on_move.h"
#include "src/utils/glm.h"
#include "src/utils/macros.h"

namespace warhol {

class Shader;
class TextureAtlas;

class MinecraftCube {
 public:
  enum class Face {
    kFront,
    kBack,
    kLeft,
    kRight,
    kTop,
    kBottom,
  };

  MinecraftCube() = default;
  ~MinecraftCube();
  DELETE_COPY_AND_ASSIGN(MinecraftCube);
  DEFAULT_MOVE_AND_ASSIGN(MinecraftCube);

  bool Init();

  // Set the textures index for the face. -1 means don't change the texture for
  // this layer.
  void SetFace(Face, int layer, Pair<Pair<float>> min_max_uvs);

  const glm::vec3& position() const { return position_; }
  void set_position(glm::vec3 pos) { position_ = pos; }

  void Render(Shader*);

 private:
  glm::vec3 position_;
  glm::mat4 model_ = glm::mat4(1.0f);

  ClearOnMove<uint32_t> vao_;
  ClearOnMove<uint32_t> vertex_vbo_;
  ClearOnMove<uint32_t> uv_vbo1_;
  ClearOnMove<uint32_t> uv_vbo2_;
  ClearOnMove<uint32_t> ebo_;

  std::vector<float> uvs1_;
  std::vector<float> uvs2_;
};

}  // namespace warhol
