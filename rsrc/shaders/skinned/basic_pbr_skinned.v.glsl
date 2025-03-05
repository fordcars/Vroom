#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 2) in int vertexMaterialId;

// Animation
layout(location = 3) in uvec4 boneIDs;  // Joint indices
layout(location = 4) in vec4 weights;   // Weights

// Matrices
uniform mat4 MVP;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 normalMatrix;

// Skin
uniform int isSkinned;
layout(std140) uniform SkinTransformBlock {
    mat4 boneTransforms[500];  // MAX_BONES_PER_SKINNED_MESH = 500
};

flat out int materialId;
out vec3 normal_cameraspace;
out vec3 lightDirection_cameraspace;
out vec3 vertexPosition_worldspace;
out vec3 eyeDirection_cameraspace;

void calculateCameraspaceStuff(vec4 position, vec4 normal, vec3 lightPosition)
{
    vertexPosition_worldspace = (modelMatrix * position).xyz;
    
    vec3 vertexPosition_cameraspace = (viewMatrix * modelMatrix * position).xyz;
    // Vector from vertex to camera
    eyeDirection_cameraspace = -vertexPosition_cameraspace;
    
    vec3 lightPosition_cameraspace = (viewMatrix * vec4(lightPosition, 1)).xyz;
    lightDirection_cameraspace = lightPosition_cameraspace - vertexPosition_cameraspace;
    
    normal_cameraspace = (normalMatrix * normal).xyz;
}

void main()
{
    vec3 lightPosition_worldspace = vec3(60, 60, 40);
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

    calculateCameraspaceStuff(position, normal, lightPosition_worldspace);
    gl_Position = MVP * position;
    materialId = vertexMaterialId;
}
