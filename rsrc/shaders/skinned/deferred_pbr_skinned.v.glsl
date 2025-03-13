#version 330 core
#define MAX_BONES_PER_SKINNED_MESH 500

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 2) in vec2 vertexTexcoord;
layout(location = 3) in int vertexMaterialId;

// Animation
layout(location = 4) in uvec4 boneIDs;  // Joint indices
layout(location = 5) in vec4 weights;   // Weights

// Matrices
uniform mat4 MVP;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 normalMatrix;

// Skin
uniform int isSkinned;
layout(std140) uniform SkinTransformBlock {
    mat4 boneTransforms[MAX_BONES_PER_SKINNED_MESH];
};

flat out int materialId;
out vec3 vertexPosition_worldspace;
out vec3 normal_cameraspace;
out vec2 texcoord;

void main()
{
    vec4 position;
    vec4 normal;

    if(isSkinned == 1)
    {
        mat4 skinMatrix = 
            weights.x * boneTransforms[boneIDs.x] +
            weights.y * boneTransforms[boneIDs.y] +
            weights.z * boneTransforms[boneIDs.z] +
            weights.w * boneTransforms[boneIDs.w];

        position = skinMatrix * vec4(vertexPosition_modelspace, 1);
        normal = skinMatrix * vec4(vertexNormal_modelspace, 0);
    }
    else
    {
        position = vec4(vertexPosition_modelspace, 1);
        normal = vec4(vertexNormal_modelspace, 0);
    }

    materialId = vertexMaterialId;
    vertexPosition_worldspace = (modelMatrix * position).xyz;
    normal_cameraspace = (normalMatrix * normal).xyz;
    texcoord = vertexTexcoord;

    gl_Position = MVP * position;
}