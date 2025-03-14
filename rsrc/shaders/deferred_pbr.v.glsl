#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 2) in vec2 vertexTexcoord;
layout(location = 3) in int vertexMaterialId;

uniform mat4 MVP;
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;

flat out int materialId;
out vec3 vertexPosition_worldspace;
out vec3 normal_cameraspace;
out vec2 texcoord;
out mat3 tbn; // Tangent space to world space matrix

// Hacky, but simple approximation of TBN matrix calculation
mat3 calculateTBN(vec3 normal_cameraspace)
{
    vec3 tangent = vec3(1.0, 0.0, 0.0); // Assume tangent aligned with x-axis
    vec3 bitangent = normalize(cross(normal_cameraspace, tangent)); // Compute bitangent
    
    // Normalize to ensure orthonormality
    if (length(bitangent) == 0.0) {
        bitangent = vec3(0.0, 1.0, 0.0); // Use an arbitrary direction if zero
    }
    tangent = normalize(cross(bitangent, normal_cameraspace)); // Recompute tangent based on bitangent

    // Construct the TBN matrix
    return mat3(tangent, bitangent, normal_cameraspace);
}

void main()
{
    vec4 position = vec4(vertexPosition_modelspace, 1);
    vec4 normal = vec4(vertexNormal_modelspace, 0);

    materialId = vertexMaterialId;
    vertexPosition_worldspace = (modelMatrix * position).xyz;
    normal_cameraspace = (normalMatrix * normal).xyz;
    texcoord = vertexTexcoord;
    tbn = calculateTBN(normal_cameraspace);

    gl_Position = MVP * position;
}