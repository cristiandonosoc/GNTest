#version 330 core

in vec4 vertex_color;

out vec4 out_color;

uniform vec4 u_color;

void main() {
  // out_color = frag_color;
  out_color = u_color;
}
