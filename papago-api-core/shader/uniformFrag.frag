#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform Uniforms {
	vec3 inColor;
} uni;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(uni.inColor, 1.0);
}