#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;

layout(binding = 0) uniform UniformBufferClass{
    mat4 view_projection;
} ubo;

layout(binding = 2) uniform SomeName{
	mat4 shadow_view_projection;
} sn;

layout(binding = 1) uniform UniformInstanceClass{
    mat4 model;
} uio;

layout(location = 1) out vec2 texture_coord;
layout(location = 2) out vec4 shadow_coord;

void main(){
    gl_Position = ubo.view_projection * uio.model * vec4(pos, 1.0);
    texture_coord = uv;
    shadow_coord = sn.shadow_view_projection * uio.model * vec4(pos, 1.0);
}