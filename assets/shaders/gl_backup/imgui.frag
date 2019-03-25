#version 330 core

precision mediump float;

uniform sampler2D u_tex_sampler0;

in vec2 frag_uv;
in vec4 frag_color;

layout (location = 0) out vec4 out_color;

void main() {
  out_color = frag_color * texture(u_tex_sampler0, frag_uv);
}
