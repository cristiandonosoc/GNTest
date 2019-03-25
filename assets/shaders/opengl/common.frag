#version 330 core

// Attributes ------------------------------------------------------------------

in vec3 in_color;
in vec2 in_uv;

out vec4 out_color;

// Uniforms --------------------------------------------------------------------

uniform sampler2D tex_sampler;

// Code ------------------------------------------------------------------------

void main() {
  out_color = vec4(in_color, 1.0f);
  // out_color = texture(tex_sampler, in_uv);
}
