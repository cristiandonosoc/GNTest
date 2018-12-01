// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <memory>

#include "src/graphics/GL/def.h"
#include "src/utils/macros.h"

namespace warhol {

class Shader;

class Texture {
 public:
  Texture(std::string path);
  ~Texture();

  Texture(Texture&&);
  Texture& operator=(Texture&&);

  // Removes a texture from a texture unit. Will disable that texture unit.
  static void Disable(GLenum tex_unit);

  // Will assert the texture is valid.
  // Will set uniforms for the given shader. The shader should be already set.
  // |tex_unit| is in which texture unit we need to supply this texture.
	void Set(Shader*, GLenum tex_unit) const;

  const std::string& path() const { return data_.path; }

  int x() const { return data_.x; }
  int y() const { return data_.y; }
  int channels() const { return data_.channels; }

	GLuint handle() const { return handle_; }
  bool valid() const { return handle_ != 0; }

 private:
  // TODO(Cristian): Move this outside of the constructor so we can call this
  //                 when we see fit.
  void Init();

  struct MovableData {
    std::string path;

    int x;
    int y;
    int channels;

  } data_;
  GLuint handle_ = 0;

  DELETE_COPY_AND_ASSIGN(Texture);
};

}  // namespace warhol
