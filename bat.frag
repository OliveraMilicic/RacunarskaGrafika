#version 330 core

out vec4 outCol;

uniform float batPos;

void main()
{
	if (gl_FragCoord.x < batPos) //pikseli
			outCol = vec4(0.0, 1.0, 0.0, 1.0);
	else 
			discard;
}