#version 330 core

in vec2 tex_coord0;
in vec2 tex_coord1;

uniform sampler2D tex0;
uniform sampler2D tex1;

out vec4 out_color;

void main() {
  vec2 mod_coord = mod(tex_coord0, vec2(1.0f, 1.0f));
  vec4 texel0 = texture(tex0, mod_coord.xy);
  out_color = texel0;
  /* out_color = vec4(0.5f, 0.2f, 0.7f, 1.0f); */
}
