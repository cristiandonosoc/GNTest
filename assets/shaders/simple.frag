#version 330 core

in vec2 tex_coord0;
in vec2 tex_coord1;

uniform sampler2D u_tex_sampler0;
uniform sampler2D u_tex_sampler1;

out vec4 out_color;

void main() {
  vec4 texel0 = texture(u_tex_sampler0, tex_coord0);
  vec4 texel1 = texture(u_tex_sampler1, tex_coord1);

  out_color = mix(texel0, texel1, texel1.a);
}
