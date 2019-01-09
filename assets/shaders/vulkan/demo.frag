#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec2 uv;

layout(binding = 1) uniform sampler2D u_sampler;

layout(location = 0) out vec4 out_color;

void main() {
    // out_color = vec4(frag_color, 1.0);
    // out_color = vec4(uv, 0.0f, 1.0f);
    out_color = texture(u_sampler, uv);
}
