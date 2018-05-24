#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec3 fragPos;


layout(binding = 0) uniform VPMatrix {
	mat4 viewProj;
} vpMat;

layout(binding = 2) uniform sampler2D tex;

layout(binding = 3) uniform Light{
	vec3 pos;
} light;


layout(location = 0) out vec4 color;

vec4 ambient()
{
	return vec4(0.1f, 0.1f, 0.1f, 1.0f);
}

vec4 diffuse()
{
	vec3 dirToLight = fragPos - (mat3(vpMat.viewProj) * light.pos);
	float intensity = dot(normalize(dirToLight), normalize(-norm)); //-norm => wrong normal?
	return clamp(intensity, 0.0f, 1.0f) * vec4(1.0f);
}

vec4 specular()
{
	return vec4(0.0f);
}


void main()
{
	float intensity = 1.0f;
	vec4 phong = ambient() + intensity * (diffuse() + specular());
	color =  texture(tex, uv) * phong;
}