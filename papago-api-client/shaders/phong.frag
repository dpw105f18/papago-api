#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 1) in vec3 norm;
layout(location = 2) in vec3 fragPos;


layout(binding = 0) uniform VPMatrix {
	mat4 viewProj;
} vpMat;

layout(binding = 2) uniform sampler2D tex;
 
layout(binding = 3) uniform LightPos {
	vec3 lightPos;
} lightPos;

layout(binding = 4) uniform ViewPos {
	vec3 viewPos;
} viewPos;

layout(binding = 5) uniform LightColor {
	vec3 lightColor;
} lightColor;

layout(binding = 6) uniform LightShinyness {
	float lightShinyness; 
} lightShinyness;

layout(location = 0) out vec4 color;

float ambient()
{
	return 0.5f;
}

float diffuse()
{
	vec3 dirToLight = (mat3(vpMat.viewProj) * lightPos.lightPos) - fragPos;
	float intensity = dot(normalize(norm), normalize(dirToLight)); 
	return clamp(intensity, 0.0f, 1.0f);
	//return vec4(lightColor.color, 1.0f);
}

float specular()
{
	vec3 dirToLight = (mat3(vpMat.viewProj) * lightPos.lightPos) - fragPos;
	vec3 dirToEye =  (mat3(vpMat.viewProj) * viewPos.viewPos) - fragPos;
	vec3 lightReflection = reflect(normalize(-dirToLight), normalize(norm));
	float intensity = clamp(dot(normalize(lightReflection), normalize(-dirToEye)), 0.0f, 1.0f);
	return pow(intensity, lightShinyness.lightShinyness);
}


void main()
{
	vec3 phong = lightColor.lightColor * (ambient() + diffuse() + specular());
	phong = clamp(phong, 0.0f, 1.0f);
	//vec4 sam = texture(tex, uv);
	//color =  vec4(sam.rgb * phong, sam.a);
	color = vec4(phong, 1.0f);
}