#version 330 core

precision mediump float;

uniform sampler2D tex;

in vec2 frag_uv;
in vec4 frag_color;

layout (location = 0) out vec4 out_color;

void main() {
  out_color = frag_color * texture(tex, frag_uv);
}
