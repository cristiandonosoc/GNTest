#version 330 core

in vec2 tex_coord0;

uniform sampler2DArray u_tex_sampler0;
uniform float u_texture_index;

out vec4 out_color;

void main() {
  // out_color = texture(u_tex_sampler0, vec3(tex_coord0, u_texture_index));
  out_color = vec4(1, 0, 0, 1);
}
