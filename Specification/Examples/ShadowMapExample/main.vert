in vec4 position;
in vec4 color;
uniform mat4 MVP;
uniform mat4 DepthBiasMVP; 

out vec4 gl_Position;
out vec4 ShadowCoord; 

void main(){
	gl_Position =  MVP * position;
	ShadowCoord = DepthBiasMVP * position;
}