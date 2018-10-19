#version 330 core

in vec3 vertex_color;
in vec2 tex_coord;

out vec4 out_color;

uniform sampler2D tex0;

void main() {
  vec4 texel = texture(tex0, tex_coord);
  if (texel.a < 0.5) {
    discard;
  } else {
    /* out_color = texel; */
    out_color = vec4(texel.a, 0.0f, texel.a, 1.0f);
  }
}


