#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 uv;

layout(binding = 0) uniform sampler2D sam;
layout(binding = 1) uniform Color {
	vec3 val;
	vec2 val2;
	float someFloat;
} in_col;

layout(location = 0) out vec4 out_col;

void main() {
	out_col = vec4(in_col.val * texture(sam, uv).xyz, 1.0);
}