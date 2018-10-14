#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_color;
layout (location = 2) in vec2 a_tex_coord;

uniform mat4 transform;

out vec3 vertex_color;
out vec2 tex_coord;

void main() {
  gl_Position = transform * vec4(a_pos, 1.0f);
  // gl_Position = vec4(a_pos, 1.0f);

  vertex_color = a_color;
  tex_coord = a_tex_coord;

}
