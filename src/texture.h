// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <memory>

#include <GL/gl3w.h>

#include "utils/macros.h"

namespace warhol {

class Shader;

class Texture {
 public:
  Texture(std::string path);
  ~Texture();

  Texture(Texture&&);
  Texture& operator=(Texture&&);

  // Will assert the texture is valid.
  // Will set uniforms for the given shader. The shader should be already set.
  // |tex_unit| is in which texture unit we need to supply this texture.
	void Use(const Shader&, GLenum tex_unit) const;

  const std::string& path() const { return data_.path; }

  int x() const { return data_.x; }
  int y() const { return data_.y; }
  int channels() const { return data_.channels; }

	GLuint handle() const { return handle_; }
  bool valid() const { return handle_ != 0; }

 private:
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
