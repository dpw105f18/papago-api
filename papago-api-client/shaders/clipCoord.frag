#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 pos;

layout(location = 0) out vec4 color;

void main() {
	color = vec4((pos.xyz / pos.w) * 0.5f + 0.5f, 1.0f);
}