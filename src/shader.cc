// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "shader.h"

#include <assert.h>

#include "src/graphics/GL/utils.h"
#include "src/utils/glm_impl.h"
#include "src/utils/log.h"

namespace warhol {

const char* Shader::Attributes::kModel = "model";


// Helpers Declarations --------------------------------------------------------

namespace {

uint32_t CompileShader(GLenum kind, const std::string& src);
typedef void(*GLMatrixFunction)(GLint, GLsizei, GLboolean, const GLfloat*);

}  // namespace

// Shader Implementation -------------------------------------------------------

Shader::Shader() = default;

Shader::Shader(std::string vert_src, std::string frag_src)
    : vert_src_(std::move(vert_src)), frag_src_(std::move(frag_src)) {}

Shader::~Shader() { Clear(); }

bool Shader::Init() {
  bool res = InternalInit();
  if (!res)
    Clear();
  return res;
}

bool Shader::InternalInit() {
  if (vert_src_.empty() || frag_src_.empty()) {
    LOG(WARNING) << "Shaders sources must be set before calling Init";
  }

  vert_handle_ = CompileShader(GL_VERTEX_SHADER, vert_src_);
  if (!vert_handle_)
    return false;

  frag_handle_ = CompileShader(GL_FRAGMENT_SHADER, frag_src_);
  if (!frag_handle_)
    return false;

  // Create the shader program.
  program_handle_ = glCreateProgram();
  if (!program_handle_) {
    LOG(ERROR) << "glCreateProgram: could not allocate a program";
    return false;
  }

  glAttachShader(*program_handle_, *vert_handle_);
  glAttachShader(*program_handle_, *frag_handle_);
  glLinkProgram(*program_handle_);

  GLint success = 0;
  glGetProgramiv(*program_handle_, GL_LINK_STATUS, &success);
  if (success == GL_FALSE) {
    GLchar log[2048];
    glGetProgramInfoLog(*program_handle_, sizeof(log), 0, log);
    LOG(ERROR) << "Could not link shader: " << log;
    return false;
  }

  ObtainAttributes();
  ObtainUniforms();

  return true;
}

void Shader::Use() {
  assert(program_handle_);
  glUseProgram(*program_handle_);
}

void Shader::Clear() {
  vert_handle_.Clear();
  frag_handle_.Clear();
  program_handle_.Clear();
}

void Shader::ObtainUniforms() {
  char buf[256];
  // Size of the longest uniform name.
  GLint max_name_size;
  glGetProgramiv(
      *program_handle_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_name_size);
  assert((uint32_t)max_name_size <= sizeof(buf));

  // Get the uniforms.
  GLint uniform_count = 0;
  glGetProgramiv(*program_handle_, GL_ACTIVE_UNIFORMS, &uniform_count);
  for (GLint i = 0; i < uniform_count; i++) {
    // Get type data.
    GLsizei length, count;
    GLenum type;
    glGetActiveUniform(
        *program_handle_, i, max_name_size, &length, &count, &type, buf);
    // Get location data.
    GLint location = glGetUniformLocation(*program_handle_, buf);
    if (location < 0) {
      LOG(ERROR) << "Could not find location for uniform: " << buf;
      assert(false);
    }

    Uniform uniform = {};
    uniform.name = buf;
    uniform.location = location;
    uniform.type = type;
    uniform.count = count;
    uniform.size = GLEnumToSize(type);
    uniforms_[uniform.name] = std::move(uniform);
  }
}

void Shader::ObtainAttributes() {
  char buf[256];
  GLint max_name_size;
  glGetProgramiv(
      *program_handle_, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_name_size);
  assert((uint32_t)max_name_size <= sizeof(buf));

  GLint attribute_count = 0;
  glGetProgramiv(*program_handle_, GL_ACTIVE_ATTRIBUTES, &attribute_count);
  for (GLint i = 0; i < attribute_count; i++) {
    GLsizei length, count;
    GLenum type;
    glGetActiveAttrib(
        *program_handle_, i, max_name_size, &length, &count, &type, buf);
    GLint location = glGetAttribLocation(*program_handle_, buf);
    if (location < 0) {
      LOG(ERROR) << "Could not find attribute for attribute: " << buf;
      assert(false);
    }

    Attribute attribute = {};
    attribute.name = buf;
    attribute.location = location;
    attribute.type = type;
    attribute.count = count;
    attribute.size = GLEnumToSize(type);
    attributes_[attribute.name] = std::move(attribute);
  }
}

const Uniform* Shader::GetUniform(const std::string& name) const {
  auto it = uniforms_.find(name);
  if (it == uniforms_.end())
    return nullptr;
  return &it->second;
}

const Attribute* Shader::GetAttribute(const std::string& name) const {
  auto it = attributes_.find(name);
  if (it == attributes_.end())
    return nullptr;
  return &it->second;
}

bool Shader::SetFloat(const std::string& name, float val) {
  const Uniform* uniform = GetUniform(name);
  if (!uniform) {
    LOG(WARNING) << "Could not find uniform: " << name;
    return false;
  }

  glUniform1f(uniform->location, val);
  return true;
}


bool Shader::SetInt(const std::string& name, int val) {
  const Uniform* uniform = GetUniform(name);
  if (!uniform) {
    LOG(WARNING) << "Could not find uniform: " << name;
    return false;
  }

  glUniform1i(uniform->location, val);
  return true;
}

bool Shader::SetMat4(const std::string& uniform_name, const glm::mat4& mat) {
  return SetMatrix(uniform_name, 4, glm::value_ptr(mat));
}

bool
Shader::SetMatrix(const std::string& uniform_name,
                  size_t mat_length,
                  const float* data) {
  const Uniform* uniform = GetUniform(uniform_name);
  if (!uniform) {
    LOG(WARNING) << "Could not find uniform: " << uniform_name;
    return false;
  }

  int loc = uniform->location;
  switch (mat_length) {
    case 2: glUniformMatrix2fv(loc, 1, GL_FALSE, data); return true;
    case 3: glUniformMatrix3fv(loc, 1, GL_FALSE, data); return true;
    case 4: glUniformMatrix4fv(loc, 1, GL_FALSE, data); return true;
    default:
      LOG(WARNING) << "Wrong matrix length: " << mat_length;
      return false;
  }
}

// Helpers Implementation ------------------------------------------------------

namespace {

uint32_t CompileShader(GLenum kind, const std::string& src) {
  uint32_t handle = glCreateShader(kind);
  if (!handle) {
    LOG(ERROR) << "Could not allocate shader.";
    return 0;
  }

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
    LOG(ERROR) << "Error compiling " << GLEnumToString(kind)
               << " shader: " << log;
    return 0;
  }
  return handle;
}

}  // namespace

}  // namespace warhol
