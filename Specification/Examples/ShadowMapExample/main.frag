in vec4 color;
uniform sampler2D shadowMap;
out vec4 outColor;

void main(){
	float visibility = 1.0;
	if ( texture( shadowMap, ShadowCoord.xy ).z  <  ShadowCoord.z){
		visibility = 0.5;
	}
	outColor = color * visibility;
}