#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 2) in vec2 vertexTexcoord;
layout(location = 3) in int vertexMaterialId;

uniform mat4 MVP;
uniform mat4 modelMatrix;

flat out int materialId;
out vec3 vertexPosition_worldspace;
out vec2 texcoord;

void main()
{
    vec4 position = vec4(vertexPosition_modelspace, 1);

    materialId = vertexMaterialId;
    vertexPosition_worldspace = (modelMatrix * position).xyz;
    texcoord = vertexTexcoord;

    gl_Position = MVP * position;
}