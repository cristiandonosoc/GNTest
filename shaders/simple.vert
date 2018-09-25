#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 pos;

layout(location = 0) out vec4 vertex_color;

void main() {
  gl_Position = vec4(pos, 1.0);
  vertex_color = vec4(0.5, 0.0, 0.0, 1.0);
}
