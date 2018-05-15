#version 450
uniform UniformBuffer{
	mat4 projection;
	mat4 view;
} uniformBuffer;


uniform mat4 model;

in vec3 inPosition;
in vec3 inColor;
in vec2 inTexCoord;

out vec3 fragColor;
out vec2 fragTexCoord;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	gl_Position = uniformBuffer.projection * uniformBuffer.view * model * vec4(inPosition, 1.0);
	fragColor = inColor;
	fragTexCoord = inTexCoord;
}
