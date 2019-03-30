// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdint.h>

#include "warhol/graphics/common/shader.h"
#include "warhol/graphics/opengl/renderer_backend.h"
#include "warhol/graphics/opengl/utils.h"
#include "warhol/utils/log.h"

namespace warhol {
namespace opengl {

namespace {

bool CompileShader(Shader* shader, const char* source, GLenum shader_kind,
                   uint32_t* out_handle) {
  uint32_t handle = glCreateShader(shader_kind);
  if (!handle) {
    LOG(ERROR) << "Shader " << shader->name << ": Could not allocate shader.";
    return false;
  }

  // Compile the shader source.
  const GLchar* gl_src = source;
  GL_CHECK(glShaderSource(handle, 1, &gl_src, 0));
  GL_CHECK(glCompileShader(handle));

  GLint success = 0;
  GL_CHECK(glGetShaderiv(handle, GL_COMPILE_STATUS, &success));
  if (success == GL_FALSE) {
    GLchar log[2048];
    GL_CHECK(glGetShaderInfoLog(handle, sizeof(log), 0, log));
    GL_CHECK(glDeleteShader(handle));
    LOG(ERROR) << "Shader " << shader->name << ": Error compiling "
               << GLEnumToString(shader_kind) << " shader: " << log;
    return false;
  }

  *out_handle = handle;
  return true;
}

bool CompileVertShader(Shader* shader, uint32_t* out_handle) {
  return CompileShader(shader, shader->vert_source.c_str(), GL_VERTEX_SHADER,
                       out_handle);
}

bool CompileFragShader(Shader* shader, uint32_t* out_handle) {
  return CompileShader(shader, shader->frag_source.c_str(), GL_FRAGMENT_SHADER,
                       out_handle);
}

// Returns the program handle or 0.
uint32_t LinkShader(uint32_t vert_handle, uint32_t frag_handle) {
  uint32_t prog_handle = glCreateProgram();
  if (prog_handle == 0) {
    LOG(ERROR) << "glCreateProgram: could not allocate a program";
    return 0;
  }

  // Link 'em.
  GL_CHECK(glAttachShader(prog_handle, vert_handle));
  GL_CHECK(glAttachShader(prog_handle, frag_handle));
  GL_CHECK(glLinkProgram(prog_handle));
  glDeleteShader(vert_handle);
  glDeleteShader(frag_handle);

  GLint success = 0;
  GL_CHECK(glGetProgramiv(prog_handle, GL_LINK_STATUS, &success));
  if (success == GL_FALSE) {
    GLchar log[2048];
    GL_CHECK(glGetProgramInfoLog(prog_handle, sizeof(log), 0, log));
    LOG(ERROR) << "Could not link shader: " << log;
    return 0;
  }

  return prog_handle;
}

// Warhol's shaders are structured into known uniform blocks, which need to be
// bound to pre-known binding indices.
bool LinkUniformBinding(const char* block_name, uint32_t prog_handle,
                        uint32_t binding) {
  uint32_t block_index = glGetUniformBlockIndex(prog_handle, block_name);
  if (block_index == GL_INVALID_INDEX) {
    LOG(WARNING) << "Could not find uniform buffer block " << block_name;
    return false;
  }

  GL_CHECK(glUniformBlockBinding(prog_handle, block_index, binding));
  return true;
}

// Create a uniform buffer and bind it.
uint32_t BindBufferBase(uint32_t binding) {
  uint32_t ubo_handle = 0;
  GL_CHECK(glGenBuffers(1, &ubo_handle));
  GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, binding, ubo_handle));
  return ubo_handle;
}

bool InternalUploadShader(Shader* shader, ShaderDescription* description) {
  *description = {};  // Clear out.

  // Compile the shaders and and program.
  uint32_t vert_handle = 0;
  uint32_t frag_handle = 0;
  if (!CompileVertShader(shader, &vert_handle) ||
      !CompileFragShader(shader, &frag_handle)) {
    return false;
  }

  uint32_t prog_handle = LinkShader(vert_handle, frag_handle);
  if (prog_handle == 0)
    return false;

  description->program_handle = prog_handle;

  // Get the uniform buffer blocks.
  if (!LinkUniformBinding("Camera", prog_handle, 0)) {
    LOG(ERROR) << "Uniform block binding is not optional.";
    return false;
  }
  description->camera_binding = 0;

  if (LinkUniformBinding("VertUniforms", prog_handle, 1)) {
    description->vert_ubo_binding = 1;
    description->vert_ubo_handle = BindBufferBase(1);
  }

  if (LinkUniformBinding("FragUniforms", prog_handle, 2)) {
    description->frag_ubo_binding = 2;
    description->frag_ubo_handle = BindBufferBase(2);
  }

  return true;
}

}  // namespace

bool UploadShader(Shader* shader, ShaderDescription* shader_desc) {
  bool result = InternalUploadShader(shader, shader_desc);
  if (!result) {
    ShutdownShader(shader_desc);
    return false;
  }

  LOG(DEBUG) << "Uploaded shader. " << std::endl
             << "[VERT] Handle: " << shader_desc->vert_ubo_handle
             << ", Binding: " << shader_desc->vert_ubo_binding << std::endl
             << "[FRAG] Handle: " << shader_desc->frag_ubo_handle
             << ", Binding: " << shader_desc->frag_ubo_binding << std::endl;

  return true;
}

void ShutdownShader(ShaderDescription* description) {
  if (description->program_handle > 0)
    GL_CHECK(glDeleteProgram(description->program_handle));

  if (description->vert_ubo_handle > 0)
    GL_CHECK(glDeleteBuffers(1, &description->vert_ubo_handle));

  if (description->frag_ubo_handle > 0)
    GL_CHECK(glDeleteBuffers(1, &description->frag_ubo_handle));

  *description = {};    // Clear.
}

}  // namespace opengl
}  // namespace warhol
