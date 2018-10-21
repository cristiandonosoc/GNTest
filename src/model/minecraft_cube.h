// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <vector>

#include "src/utils/glm.h"

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

  MinecraftCube(TextureAtlas*);
  bool Init();

  // Set the textures index for the face. -1 means don't change the texture for
  // this layer.
  void SetFace(Face, int index1, int index2 = -1);

  // Will load the atlas as the default texture.
  void SetTextures(Shader*) const;

  const glm::vec3& position() const { return position_; }
  void set_position(glm::vec3 pos) {
    position_ = pos;
    dirty_ = true;
  }

  void Render(Shader*);

 private:
  TextureAtlas* atlas_;  // Not owning. Must outlive.

  glm::vec3 position_;
  glm::mat4 model_ = glm::mat4(1.0f);

  uint32_t vao_;
  uint32_t vertex_vbo_;
  uint32_t uv_vbo1_;
  uint32_t uv_vbo2_;
  uint32_t ebo_;

  std::vector<float> uvs1_;
  std::vector<float> uvs2_;

  bool dirty_ = true;
};

}  // namespace warhol
