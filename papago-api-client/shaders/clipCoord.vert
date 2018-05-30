#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;

layout(binding = 0) uniform VpMatrix {
	mat4 vp;
} view_proj;

layout(binding = 1) uniform ModelMatrix {
	mat4 m;
} model;

layout(location = 0) out vec4 out_pos;

void main() {
	gl_Position = view_proj.vp * model.m * vec4(pos, 1.0f);
	out_pos = gl_Position;
}