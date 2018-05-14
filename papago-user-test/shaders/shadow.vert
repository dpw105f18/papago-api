#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;

layout(binding = 0) uniform UniformBufferClass{
    mat4 view_projection;
    mat4 shadow_view_projection;
};
layout(binding = 1) uniform UniformInstanceClass{
    mat4 model;
};

layout(location = 0) out vec2 texture_coord;
layout(location = 1) out vec4 shadow_coord;

void main(){
    gl_Position = view_projection * model * vec4(pos, 1.0);
    texture_coord = uv;
    shadow_coord = shadow_view_projection * model * vec4(pos, 1.0);
}
