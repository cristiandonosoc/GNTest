#version 330 core

in vec3 vertex_color;
in vec2 tex_coord;

out vec4 out_color;

uniform vec4 u_color;
uniform sampler2D tex;

void main() {
  // out_color = frag_color;
  // out_color = u_color;
  // out_color = vec4(vertex_color, 1.0f);
  out_color = texture(tex, tex_coord);
  // out_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
