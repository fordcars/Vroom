#version 330 core

uniform vec3 color;

in vec3 fScreenPos;
out vec3 outColor;

void main()
{
    outColor = vec3(1.0f, 0.5f, 0.0f);
}