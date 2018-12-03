#version 330 core

in vec2 tex_coord0;

uniform sampler2D u_tex_sampler0;

out vec4 out_color;

void main() {
  out_color = texture(u_tex_sampler0, tex_coord0);
}
