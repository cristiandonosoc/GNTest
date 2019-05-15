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
  vec2 uv = gl_FragCoord.xy / uniforms.screen_size.y;
  color = vec4(uv.xy, 0, 1);

  /* if (gl_FragCoord.y < 200) */
  /*   color = vec4(1, 0, 0, 1); */
  /* else */
  /*   color = vec4(0, 1, 0, 1); */

  /* float i = gl_FragCoord.x / uniforms.screen_size.x; */
  // color = vec4(i * uniforms.sky_color1 + (1 - i) * uniforms.sky_color2, 1.0f);
  // color = vec4(1.0f, 0.0f, 1.0f, 1.0f);
}
