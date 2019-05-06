#version 330 core
#extension GL_ARB_separate_shader_objects : enable

// Pass through vertex shader.
// Meant to be used for screen processing fragment shaders.

// Attributes ------------------------------------------------------------------

layout (location = 0) in vec3 in_pos;

// Uniforms --------------------------------------------------------------------

layout (std140) uniform Camera {
  mat4 proj;
  mat4 view;
} camera;

// Code ------------------------------------------------------------------------

void main() {
  gl_Position = camera.proj * camera.view * uniforms.model * vec4(in_pos, 1.0);
}
