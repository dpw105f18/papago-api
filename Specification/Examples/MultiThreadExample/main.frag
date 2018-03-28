#version 450
uniform sampler2D texture;

in vec3 fragColor;
in vec2 fragTexCoord;

out vec4 outColor;

void main() {
	outColor = vec4(fragColor * texture(texSampler, fragTexCoord).rgb, 1.0f);
}
