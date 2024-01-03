#version 330 core 

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec3 inCol;
layout(location = 2) in float inAlpha;

//uniform vec2 pos;

void main()
{
	gl_Position = vec4(inPos.x, inPos.y, 0.0, 1.0);
}