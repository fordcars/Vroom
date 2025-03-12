#version 330 core

layout(location = 0) in vec3 vertexPosition_clipspace;
layout(location = 1) in vec2 vertexTexcoord;

out vec2 texcoord;

void main()
{
	gl_Position = vec4(vertexPosition_clipspace, 1);
	texcoord = vertexTexcoord;
}