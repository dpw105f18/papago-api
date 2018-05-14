#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 pos;

//TODO: Change the uniforms to a more sensible layout
layout(binding = 0) uniform UniformBuffer {
	mat4 view_projection_matrix;
} uniforms;

layout(binding = 1) uniform InstanceUniformBuffer {
	mat4 model_matrix;
} instance;

void main() {
	gl_Position = uniforms.view_projection_matrix * instance.model_matrix * vec4(pos, 1);
}
