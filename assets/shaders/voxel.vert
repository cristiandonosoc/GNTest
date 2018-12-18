#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex_coord0;
layout (location = 2) in float a_tex_index;
layout (location = 3) in vec3 a_face_color;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec2 tex_coord0;
out float tex_index;
out vec3 face_color;

void main() {
  tex_coord0 = a_tex_coord0;

  gl_Position = u_projection * u_view * u_model * vec4(a_pos, 1.0f);
  tex_index = a_tex_index;
  face_color = a_face_color;
}
