// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <GL/gl3w.h>

#include "shader.h"

namespace warhol {

Shader::Shader() = default;

Shader::Shader(std::string vert_src, std::string frag_src)
    : vert_src_(std::move(vert_src)), frag_src_(std::move(frag_src)) {}

Status
Shader::Init() {
  if (vert_src_.empty())
    return Status("Shaders must be set before init");

  return Status::Ok();
}

Shader::~Shader() {
  if (vert_handle_)
    glDeleteShader(vert_handle_);
  if (frag_handle_)
    glDeleteShader(frag_handle_);
  if (handle_)
    glDeleteShader(handle_);
}

}  // namespace warhol
