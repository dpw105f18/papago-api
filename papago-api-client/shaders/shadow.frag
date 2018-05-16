#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 3) uniform sampler2D tex;
layout(binding = 4) uniform sampler2D shadow_map;

layout(location = 0) in vec2 texture_coord;
layout(location = 1) in vec4 shadow_coord;

layout(location = 0) out vec4 color;

float bias = 0.005;

void main(){
    float visibility = texture(shadow_map, shadow_coord.xy).z < shadow_coord.z - bias
        ? 0.5 
        : 1.0;
    color = texture(tex, texture_coord) * visibility;
}
