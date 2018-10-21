#version 330 core

in vec2 tex_coord0;
in vec2 tex_coord1;

uniform sampler2D tex0;
uniform sampler2D tex1;

out vec4 out_color;

void main() {
  vec4 texel0 = texture(tex0, tex_coord0);
  vec4 texel1 = texture(tex1, tex_coord1);

  out_color = mix(texel0, texel1, texel1.a);
  // out_color =  texel1;
  // out_color = mix(texture(tex0, tex_coord0), texture(tex1, tex_coord1), texel0.a);
  // out_color = texel0 * texel1;
}
