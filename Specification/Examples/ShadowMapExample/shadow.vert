#version 450 core
in vec4 position;
uniform mat4 depthMVP;

void main(){
 gl_Position =  depthMVP * position;
}

