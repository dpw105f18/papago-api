#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 pos;

layout(binding = 0) uniform VPMatrix {
	mat4 viewProj;
} vpMat;

layout(binding = 1) uniform MMatrix {
	mat4 model;
} mMat;


layout(location = 1) out vec3 out_norm;
layout(location = 2) out vec3 fragPos;

void main()
{
	mat4 mvp = vpMat.viewProj * mMat.model;
	vec4 vertPos = mvp * vec4(pos, 1.0f); 

	gl_Position = vertPos;
	out_norm = (mvp * vec4(pos, 1.0f)).xyz;
	fragPos = vertPos.xyz;
}