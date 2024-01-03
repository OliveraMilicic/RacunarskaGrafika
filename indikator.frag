#version 330 core

out vec4 outCol;

uniform int swOnOff;

void main()
{
	if (swOnOff == 1) {
		outCol = vec4(1.0, 0.0, 0.0, 1.0);
	} 
	else
	{ 
		outCol = vec4(0.5, 0.0, 0.0, 1.0);
	}
}