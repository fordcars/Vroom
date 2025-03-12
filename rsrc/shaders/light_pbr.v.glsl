#version 330 core

layout(location = 0) in vec3 vertexPosition_clipspace;
layout(location = 1) in vec2 vertexTexcoord;

uniform mat4 viewMatrix;
uniform vec3 lightPos_worldspace;

out vec3 lightPos_cameraspace;
out vec2 texcoord;

void main()
{
	gl_Position = vec4(vertexPosition_clipspace, 1);
	lightPos_cameraspace = (viewMatrix * vec4(lightPos_worldspace, 1)).xyz;

	texcoord = vertexTexcoord;
}