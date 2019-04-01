#version 330 core
#extension GL_ARB_separate_shader_objects : enable

// NOTE: //# format is a warhol specific extension.

//#INCLUDE "common.gl"

// TODO(Cristian): Continue with the compilation eventually.
#define UNIFORM_BLOCK(block_name) layout(std140) uniform block_name

// Attributes ------------------------------------------------------------------

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_color;
layout (location = 2) in vec2 in_uv;

out vec3 color;
out vec2 uv;

// Uniforms --------------------------------------------------------------------

layout (std140) uniform Camera {
  mat4 proj;
  mat4 view;
} camera;

UNIFORM_BLOCK(VertUniforms) {
  mat4 model;
} uniforms;

// Code ------------------------------------------------------------------------

void main() {
  gl_Position = camera.proj * camera.view * uniforms.model * vec4(in_pos, 1.0);
  color = in_pos;
  uv = in_uv;
}
