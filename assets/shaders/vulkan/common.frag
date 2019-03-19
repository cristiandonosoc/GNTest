// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

layout(binding = 1) uniform UBO_Fragment {
  sampler2D tex0;
  sampler2D tex1;
} uniforms;

void main() {
  out_color = texture(uniforms.tex0, in_uv);
}
