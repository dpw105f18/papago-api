#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 pos;
layout(binding = 0) uniform mat4 view_projection_matrix;
layout(binding = 1) uniform mat4 model_matrix;

void main() {
	gl_Position = view_projection_matrix * model_matrix * vec4(pos, 1);
}