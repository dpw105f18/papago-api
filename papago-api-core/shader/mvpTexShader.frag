#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 uv;

layout(binding = 2) uniform sampler2D sam;

layout(location = 0) out vec4 col;
void main() {
	col = texture(sam, uv);
}