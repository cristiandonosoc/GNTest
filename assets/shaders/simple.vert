#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_color;

uniform float u_x_offset;

out vec3 vertex_color;

void main() {
  vec3 pos = vec3(a_pos.x + u_x_offset, -a_pos.y, a_pos.z);
  gl_Position = vec4(pos, 1.0f);

  vertex_color = pos;
}
