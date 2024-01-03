#version 330 core

out vec4 outCol;

in vec2 chTex;

uniform sampler2D uTex;


void main()
{
	outCol = texture(uTex, chTex);
}