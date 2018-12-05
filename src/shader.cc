// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "shader.h"

#include <assert.h>

#include "src/assets.h"
#include "src/graphics/GL/utils.h"
#include "src/utils/file.h"
#include "src/utils/glm_impl.h"
#include "src/utils/log.h"

namespace warhol {

ShaderString Shader::Attribute::kPos = {"a_pos"};
ShaderString Shader::Attribute::kColor = {"a_color"};
ShaderString Shader::Attribute::kTexCoord0 = {"a_tex_coord0"};
ShaderString Shader::Attribute::kTexCoord1 = {"a_tex_coord1"};
ShaderString Shader::Attribute::kTexIndex = {"a_tex_index"};

ShaderString Shader::Uniform::kModel = {"u_model"};
ShaderString Shader::Uniform::kView = {"u_view"};
ShaderString Shader::Uniform::kProjection = {"u_projection"};
ShaderString Shader::Uniform::kTexSampler0 = {"u_tex_sampler0"};
ShaderString Shader::Uniform::kTexSampler1 = {"u_tex_sampler1"};
ShaderString Shader::Uniform::kTexSampler2 = {"u_tex_sampler2"};
ShaderString Shader::Uniform::kTexSampler3 = {"u_tex_sampler3"};
ShaderString Shader::Uniform::kTexSampler4 = {"u_tex_sampler4"};
ShaderString Shader::Uniform::kTexSampler5 = {"u_tex_sampler5"};
ShaderString Shader::Uniform::kTexSampler6 = {"u_tex_sampler6"};
ShaderString Shader::Uniform::kTexSampler7 = {"u_tex_sampler7"};
ShaderString Shader::Uniform::kTexSampler8 = {"u_tex_sampler8"};
ShaderString Shader::Uniform::kTexSampler9 = {"u_tex_sampler9"};
ShaderString Shader::Uniform::kTexSampler10 = {"u_tex_sampler10"};
ShaderString Shader::Uniform::kTexSampler11 = {"u_tex_sampler11"};
ShaderString Shader::Uniform::kTexSampler12 = {"u_tex_sampler12"};
ShaderString Shader::Uniform::kTexSampler13 = {"u_tex_sampler13"};
ShaderString Shader::Uniform::kTexSampler14 = {"u_tex_sampler14"};
ShaderString Shader::Uniform::kTexSampler15 = {"u_tex_sampler15"};


// Helpers Declarations --------------------------------------------------------

namespace {

uint32_t
CompileShader(const std::string& name, GLenum kind, const std::string& src);
typedef void (*GLMatrixFunction)(GLint, GLsizei, GLboolean, const GLfloat*);

}  // namespace

// Shader Implementation -------------------------------------------------------

Shader
Shader::FromAssetPath(std::string name, std::string vert, std::string frag) {
  std::vector<char> vert_data, frag_data;
  if (!ReadWholeFile(Assets::ShaderPath(std::move(vert)), &vert_data) ||
      !ReadWholeFile(Assets::ShaderPath(std::move(frag)), &frag_data)) {
    return Shader();
  }

  Shader shader(std::move(name), vert_data.data(), frag_data.data());
  if (!shader.Init())
    return Shader();
  return shader;
}

Shader::Shader() = default;

Shader::Shader(std::string name, std::string vert_src, std::string frag_src)
    : name_(std::move(name)),
      vert_src_(std::move(vert_src)),
      frag_src_(std::move(frag_src)) {}

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

  vert_handle_ = CompileShader(name_, GL_VERTEX_SHADER, vert_src_);
  if (!vert_handle_)
    return false;

  frag_handle_ = CompileShader(name_, GL_FRAGMENT_SHADER, frag_src_);
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
      LOG(ERROR) << "Shader " << name_
                 << ": Could not find attribute for attribute: " << buf;
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

const Shader::Attribute* Shader::GetAttribute(ShaderString name) const {
  auto it = attributes_.find(name.value);
  if (it == attributes_.end()) {
    LOG(WARNING) << "Shader " << name_
                 << ": Could not get attribute: " << name.value;
    return nullptr;
  }
  return &it->second;
}

const Shader::Uniform* Shader::GetUniform(ShaderString name) const {
  auto it = uniforms_.find(name.value);
  if (it == uniforms_.end()) {
    LOG(WARNING) << "Shader " << name_
                 << ": Could not get uniform: " << name.value;
    return nullptr;
  }
  return &it->second;
}

bool Shader::SetFloat(ShaderString name, float val) {
  const Uniform* uniform = GetUniform(name);
  if (!uniform)
    return false;

  GL_CALL(glUniform1f, uniform->location, val);
  return true;
}


bool Shader::SetInt(ShaderString name, int val) {
  const Uniform* uniform = GetUniform(name);
  if (!uniform)
    return false;

  CTX_GL_CALL(glUniform1i, uniform->location, val);
  return true;
}

bool Shader::SetMat4(ShaderString name, const glm::mat4& mat) {
  return SetMatrix(name, 4, glm::value_ptr(mat));
}

bool Shader::SetMatrix(ShaderString name,
                  size_t mat_length,
                  const float* data) {
  const Uniform* uniform = GetUniform(name);
  if (!uniform)
    return false;

  int loc = uniform->location;
  switch (mat_length) {
    case 2: GL_CALL(glUniformMatrix2fv, loc, 1, GL_FALSE, data); return true;
    case 3: GL_CALL(glUniformMatrix3fv, loc, 1, GL_FALSE, data); return true;
    case 4: GL_CALL(glUniformMatrix4fv, loc, 1, GL_FALSE, data); return true;
    default:
      LOG(WARNING) << "Shader " << name_
                   << ": Wrong matrix length: " << mat_length;
      return false;
  }
}

// Helpers Implementation ------------------------------------------------------

namespace {

uint32_t
CompileShader(const std::string& name, GLenum kind, const std::string& src) {
  uint32_t handle = glCreateShader(kind);
  if (!handle) {
    LOG(ERROR) << "Shader " << name << ": Could not allocate shader.";
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
    LOG(ERROR) << "Shader " << name << ": Error compiling "
               << GLEnumToString(kind) << " shader: " << log;
    return 0;
  }
  return handle;
}

}  // namespace

std::pair<int, ShaderString> TextureUnitToUniform(GLenum tex) {
  switch (tex) {
    case GL_TEXTURE0: return {0, Shader::Uniform::kTexSampler0};
    case GL_TEXTURE1: return {1, Shader::Uniform::kTexSampler1};
    case GL_TEXTURE2: return {2, Shader::Uniform::kTexSampler2};
    case GL_TEXTURE3: return {3, Shader::Uniform::kTexSampler3};
    case GL_TEXTURE4: return {4, Shader::Uniform::kTexSampler4};
    case GL_TEXTURE5: return {5, Shader::Uniform::kTexSampler5};
    case GL_TEXTURE6: return {6, Shader::Uniform::kTexSampler6};
    case GL_TEXTURE7: return {7, Shader::Uniform::kTexSampler7};
    case GL_TEXTURE8: return {8, Shader::Uniform::kTexSampler8};
    case GL_TEXTURE9: return {9, Shader::Uniform::kTexSampler9};
    case GL_TEXTURE10: return {10, Shader::Uniform::kTexSampler10};
    case GL_TEXTURE11: return {11, Shader::Uniform::kTexSampler11};
    case GL_TEXTURE12: return {12, Shader::Uniform::kTexSampler12};
    case GL_TEXTURE13: return {13, Shader::Uniform::kTexSampler13};
    case GL_TEXTURE14: return {14, Shader::Uniform::kTexSampler14};
    case GL_TEXTURE15: return {15, Shader::Uniform::kTexSampler15};
    default:
      LOG(ERROR) << "Invalid texture given: " << GLEnumToString(tex);
      assert(false);
      return {0, ShaderString()};
  }
}

}  // namespace warhol
