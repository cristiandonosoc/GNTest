// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <assert.h>

#include <GL/gl3w.h>

#include "shader.h"

namespace warhol {

// Helpers Declarations --------------------------------------------------------

namespace {
Status CompileShader(GLenum kind, const std::string& src, int* handle);
}  // namespace

// Shader Implementation -------------------------------------------------------

Shader::Shader() = default;

Shader::Shader(std::string vert_src, std::string frag_src)
    : vert_src_(std::move(vert_src)), frag_src_(std::move(frag_src)) {}

Shader::~Shader() { Clear(); }

Status Shader::Init() {
  Status res = InternalInit();
  if (!res.ok())
    Clear();
  return res;
}

Status
Shader::InternalInit() {
  if (vert_src_.empty() || frag_src_.empty())
    return Status("Shaders sources must be set before calling Init");

  Status res = CompileShader(GL_VERTEX_SHADER, vert_src_, &vert_handle_);
  if (!res.ok())
    return res;
  res = CompileShader(GL_FRAGMENT_SHADER, frag_src_, &frag_handle_);
  if (!res.ok())
    return res;

  // Create the shader program.
  handle_ = glCreateProgram();
  if (!handle_)
    return Status("glCreateProgram: could not allocate a program");

  glAttachShader(handle_, vert_handle_);
  glAttachShader(handle_, frag_handle_);
  glLinkProgram(handle_);

  GLint success = 0;
  glGetProgramiv(handle_, GL_LINK_STATUS, &success);
  if (success == GL_FALSE) {
    GLchar log[2048];
    glGetProgramInfoLog(handle_, sizeof(log), 0, log);
    return Status("Could not link shader: %s", log);
  }

  // TODO: Obtain uniforms.
  // TODO: Obtain attributes.

  return Status::Ok();
}

void Shader::Use() {
  assert(handle_ != 0);
  glUseProgram(handle_);
}

void Shader::Clear() {
  if (vert_handle_)
    glDeleteShader(vert_handle_);
  if (frag_handle_)
    glDeleteShader(frag_handle_);
  if (handle_)
    glDeleteShader(handle_);
}

// Helpers Implementation ------------------------------------------------------

namespace {

Status CompileShader(GLenum kind, const std::string& src, int* out) {
  int handle = glCreateShader(kind);
  if (!handle)
    return Status("Could not allocate a shader");

  // Compile the shader source.
  const GLchar* gl_src = src.data();
  glShaderSource(handle, 1, &gl_src, 0);
  glCompileShader(handle);

  GLint success = 0;
  glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
  if (success == GL_FALSE) {
    GLchar log[2048];
    glGetShaderInfoLog(handle, sizeof(log), 0, log);
    glDeleteShader(handle);
    return Status("Error compiling shader: %s", log);
  }
  *out = handle;
  return Status::Ok();
}

}  // namespace

}  // namespace warhol
