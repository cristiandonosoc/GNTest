#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex_coord0;
layout (location = 2) in vec2 a_tex_coord1;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec2 tex_coord0;
out vec2 tex_coord1;

void main() {
  tex_coord0 = a_tex_coord0;
  tex_coord1 = a_tex_coord1;
  gl_Position = u_projection * u_view * u_model * vec4(a_pos, 1.0f);
}
