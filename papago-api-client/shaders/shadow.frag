#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 2) uniform SomeName{
	mat4 shadow_view_projection;
} sn;

layout(binding = 3) uniform sampler2D tex;
layout(binding = 4) uniform sampler2D shadow_map;

layout(location = 0) in vec2 texture_coord;
layout(location = 1) in vec4 model_pos;

layout(location = 0) out vec4 color;

float bias = 0.005f; //ori: 0.005

void main(){

	vec2 shadow_uv_coord = (model_pos.xy / model_pos.w) * 0.5f + 0.5f;
	vec4 shadow = texture(shadow_map, shadow_uv_coord);
    float visibility = shadow.r < (model_pos.z / model_pos.w) - bias
        ? 0.3 
        : 1.0;
    color = texture(tex, texture_coord) * visibility;
	//color = vec4(shadow.r, bias, model_pos.z, 1.0f);
	//color = vec4(shadow.xyz, 1.0f);
}
