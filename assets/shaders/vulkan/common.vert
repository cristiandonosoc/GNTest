// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec2 out_uv;

layout(binding = 0) uniform UBO_Vertex {
  /* vec4 kModelMatrixX; */
  /* vec4 kModelMatrixY; */
  /* vec4 kModelMatrixZ; */
  /* vec4 kModelMatrixW; */
  mat4 kModelMatrix;

  /* vec4 kViewMatrixX; */
  /* vec4 kViewMatrixY; */
  /* vec4 kViewMatrixZ; */
  /* vec4 kViewMatrixW; */
  mat4 kViewMatrix;

  /* vec4 kProjectionMatrixX; */
  /* vec4 kProjectionMatrixY; */
  /* vec4 kProjectionMatrixZ; */
  /* vec4 kProjectionMatrixW; */
  mat4 kProjectionMatrix;

  vec4 kTex0U;
  vec4 kTex0V;

  vec4 kTex1U;
  vec4 kTex1V;

  vec4 kUser0;
  vec4 kUser1;
  vec4 kUser2;
  vec4 kUser3;
  vec4 kUser4;
  vec4 kUser5;
  vec4 kUser6;
  vec4 kUser7;
} uniforms;

void main() {
  out_color = a_color;
  out_uv = in_uv;

  vec4 pos = vec4(in_pos, 1.0f);
  gl_Position = uniforms.kProjectionMatrix *
                uniforms.kViewMatrix *
                uniforms.kModelMatrix *
                pos;
}
