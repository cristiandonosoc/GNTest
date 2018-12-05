#version 330 core

in vec2 tex_coord0;
in float tex_index; uniform sampler2DArray u_tex_sampler0; // uniform sampler2D u_tex_sampler1;

out vec4 out_color;

void main() {
  vec4 texel0 = texture(u_tex_sampler0, vec3(tex_coord0, tex_index));
  out_color = texel0;
  /* out_color = vec4(0.5f, 0.2f, 0.7f, 1.0f); */
}
