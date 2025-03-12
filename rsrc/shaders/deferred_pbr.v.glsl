#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 2) in int vertexMaterialId;

uniform mat4 MVP;
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;

flat out int materialId;
out vec3 vertexPosition_worldspace;
out vec3 normal_cameraspace;

void main()
{
    vec4 position = vec4(vertexPosition_modelspace, 1);
    vec4 normal = vec4(vertexNormal_modelspace, 0);

    materialId = vertexMaterialId;
    vertexPosition_worldspace = (modelMatrix * position).xyz;
    normal_cameraspace = (normalMatrix * normal).xyz;

    gl_Position = MVP * position;
}