// Attributes ------------------------------------------------------------------

layout (location = 0) in vec2 in_pos;
layout (location = 1) in vec4 in_color;

out vec4 color;

// Uniforms --------------------------------------------------------------------

layout (std140) uniform Camera {
  mat4 proj;
  mat4 view;
} camera;

// Code ------------------------------------------------------------------------

void main() {
  gl_Position = camera.proj * camera.view * vec4(in_pos.xy, 0, 1.0);
  color = in_color;
}
