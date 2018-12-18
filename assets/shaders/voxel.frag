#version 330 core

in vec2 tex_coord0;
in float tex_index;
in vec3 face_color;

uniform sampler2DArray u_tex_sampler0;

out vec4 out_color;

void main() {
  float sum = face_color.x + face_color.y + face_color.z;
  if (sum > 0.0f) {
    out_color = vec4(face_color.xyz * 0.05f, 1);
  } else {
    vec4 texel0 = texture(u_tex_sampler0, vec3(tex_coord0, tex_index));
    out_color = texel0;
    /* out_color = vec4(0.5f, 0.2f, 0.7f, 1.0f); */
  }
}
