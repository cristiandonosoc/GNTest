// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/ui/imgui/imgui_shaders.h"

#include "warhol/graphics/common/shader.h"

namespace warhol {
namespace imgui {

// OpenGL ----------------------------------------------------------------------

#ifdef WARHOL_OPENGL_ENABLED

const char kOpenGLVertex[] = R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

// Attributes ------------------------------------------------------------------

layout (location = 0) in vec2 in_pos;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec4 in_color;

out vec4 color;
out vec2 uv;

// Uniforms --------------------------------------------------------------------

layout (std140) uniform Camera {
  mat4 proj;
  mat4 view;
} camera;

void main() {
  gl_Position = camera.proj * camera.view * vec4(in_pos.xy, 0, 1.0f);
  color = in_color;
  uv = in_uv;
}
)";

const char kOpenGLFragment[] = R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

// Attributes ------------------------------------------------------------------

in vec4 color;
in vec2 uv;

layout (location = 0) out vec4 out_color;

// Uniforms --------------------------------------------------------------------

uniform sampler2D tex_sampler;

// Code ------------------------------------------------------------------------

void main() {
  out_color = color * texture(tex_sampler, uv);
}
)";

Shader GetOpenGLImguiShader() {
  Shader shader;
  shader.name = "Imgui Shader";
  shader.vert_ubo_size = 0;
  shader.frag_ubo_size = 0;
  shader.texture_count = 1;

  shader.uuid = GetNextShaderUUID();
  shader.vert_source = kOpenGLVertex;
  shader.frag_source = kOpenGLFragment;

  return shader;
}

#else
Shader GetOpenGLImguiShader() {
  NOT_REACHED("OpenGL support not enabled.");
  return {};
}
#endif

}  // namespace imgui
}  // namespace warhol

