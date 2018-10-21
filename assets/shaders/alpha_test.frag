#version 330 core

in vec3 vertex_color;
in vec2 tex_coord0;

out vec4 out_color;

uniform sampler2D tex0;

void main() {
  vec4 texel = texture(tex0, tex_coord0);
  if (texel.a < 0.5)
    discard;
  out_color = texel;
}


