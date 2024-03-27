#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in int vertexMaterialId;

uniform mat4 MVP;
flat out int materialId;

void main()
{
    vec4 v = vec4(vertexPosition_modelspace, 1);
    gl_Position = MVP * v;

    materialId = vertexMaterialId;
}