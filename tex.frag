#version 330 core

out vec4 outCol;

in vec2 chTex;

uniform sampler2D uTex1;
uniform sampler2D uTex2;
uniform int usetexture = 0;


void main()
{
        if (usetexture == 0)
            outCol = texture(uTex1, chTex);
        if (usetexture == 1)
            outCol = texture(uTex2, chTex);
}