#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 pos;

layout(binding = 0) uniform Matrices {
	mat4 mvp;
}

void main() {
	gl_Position = mvp * pos;
}