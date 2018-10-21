#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex_coord0;
layout (location = 2) in vec2 a_tex_coord1;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 tex_coord0;
out vec2 tex_coord1;

void main() {
  gl_Position = projection * view * model * vec4(a_pos, 1.0f);

  tex_coord0 = a_tex_coord0;
  tex_coord1 = a_tex_coord1;
}
