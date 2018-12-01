#version 330 core

precision mediump float;

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_tex_coord0;
layout (location = 2) in vec4 a_color;

uniform mat4 u_projection;

out vec2 frag_uv;
out vec4 frag_color;

void main() {
  frag_uv = a_tex_coord0;
  frag_color = a_color;
  gl_Position = u_projection * vec4(a_pos.xy, 0, 1);
}
