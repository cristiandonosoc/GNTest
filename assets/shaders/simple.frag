#version 330 core

in vec3 vertex_color;
in vec2 tex_coord;

out vec4 out_color;

uniform vec4 u_color;

uniform sampler2D tex0;
uniform sampler2D tex1;

void main() {
  // out_color = frag_color;
  // out_color = u_color;
  // out_color = vec4(vertex_color, 1.0f);
  out_color = mix(texture(tex0, tex_coord), texture(tex1, tex_coord), 0.2);
  // out_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
