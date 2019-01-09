#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec2 a_uv;

layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec2 uv;

layout(binding = 0) uniform UBO {
  mat4 model;
  mat4 view;
  mat4 proj;
} ubo;

void main() {
    // gl_Position = vec4(a_pos.xy, 0.0, 1.0);
    vec4 pos = vec4(a_pos, 1.0f);
    gl_Position = ubo.proj * ubo.view * ubo.model * pos;
    frag_color = a_color;
    uv = a_uv;
}
