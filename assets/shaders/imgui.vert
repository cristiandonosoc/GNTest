#version 330 core

precision mediump float;

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_uv;
layout (location = 2) in vec4 a_color;

uniform mat4 u_proj;

out vec2 frag_uv;
out vec4 frag_color;

void main() {
  frag_uv = a_uv;
  frag_color = a_color;
  gl_Position = u_proj * vec4(a_pos.xy, 0, 1);
}
