#version 330 core
#extension GL_ARB_separate_shader_objects : enable

// Attributes ------------------------------------------------------------------

in vec3 color;
in vec2 uv;

out vec4 out_color;

// Uniforms --------------------------------------------------------------------

uniform sampler2D tex_sampler;

// Code ------------------------------------------------------------------------

void main() {
  // out_color = vec4(color, 1.0f);
  out_color = vec4(color, 1.0f) * texture(tex_sampler, uv);
}
