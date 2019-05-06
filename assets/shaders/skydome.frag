#version 330 core
#extension GL_ARB_separate_shader_objects : enable

// Attributes ------------------------------------------------------------------

out vec4 color;

// Uniforms --------------------------------------------------------------------

layout (std140) uniform FragUniforms {
  ivec2 screen_size;
  vec3 sky_color1;
  vec3 sky_color2;
} uniforms;

// Code ------------------------------------------------------------------------

void main() {
  float i = gl_FragCoord.x / screen_size.x;
  color = vec4(i * sky_color1 + (1 - i) * sky_color2, 1.0f);
}
