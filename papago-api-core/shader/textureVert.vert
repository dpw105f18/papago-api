#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
    vec4 gl_Position;
};

layout(location = 0) out vec2 uv;

layout(location = 0) in vec2 position;

vec2 uvs[3] = vec2[](
	vec2(0.5, 1.0),
	vec2(1.0, 0.0),
	vec2(0.0, 0.0)
);

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
	uv = uvs[gl_VertexIndex%3];
}