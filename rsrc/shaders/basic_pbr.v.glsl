#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 2) in int vertexMaterialId;

uniform mat4 MVP;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 normalMatrix;

flat out int materialId;
out vec3 normal_cameraspace;
out vec3 lightDirection_cameraspace;
out vec3 vertexPosition_worldspace;
out vec3 eyeDirection_cameraspace;

void main()
{
    vec3 lightPosition_worldspace = vec3(60, 60, 40);
    vertexPosition_worldspace = (modelMatrix * vec4(vertexPosition_modelspace, 1)).xyz;
    
    vec3 vertexPosition_cameraspace = (viewMatrix * modelMatrix * vec4(vertexPosition_modelspace, 1)).xyz;
    // Vector from vertex to camera
    eyeDirection_cameraspace = vec3(0, 0, 0) - vertexPosition_cameraspace;
    
    vec3 lightPosition_cameraspace = (viewMatrix * vec4(lightPosition_worldspace, 1)).xyz;
    lightDirection_cameraspace = lightPosition_cameraspace + eyeDirection_cameraspace; // Vector from vertex to light
    
    normal_cameraspace = (normalMatrix * modelMatrix * vec4(vertexNormal_modelspace, 0.0)).xyz;
    
    // Output position of the vertex
    gl_Position = MVP * vec4(vertexPosition_modelspace, 1);

    materialId = vertexMaterialId;
}