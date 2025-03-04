#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 2) in int vertexMaterialId;

// Animation
layout(location = 3) in ivec4 boneIDs;  // Joint indices
layout(location = 4) in vec4 weights;   // Weights

// Matrices
uniform mat4 MVP;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 normalMatrix;

// Uniform Block for Bone Matrices
layout(std140) uniform SkeletonTransformBlock {
    mat4 skeletonTransforms[500];  // MAX_BONES = 500
};

flat out int materialId;
out vec3 normal_cameraspace;
out vec3 lightDirection_cameraspace;
out vec3 vertexPosition_worldspace;
out vec3 eyeDirection_cameraspace;

void main()
{
    vec3 lightPosition_worldspace = vec3(60, 60, 40);

    mat4 skinMatrix = 
          weights.x * skeletonTransforms[boneIDs.x] +
          weights.y * skeletonTransforms[boneIDs.y] +
          weights.z * skeletonTransforms[boneIDs.z] +
          weights.w * skeletonTransforms[boneIDs.w];

    vec4 skinnedPosition = skinMatrix * vec4(vertexPosition_modelspace, 1.0);
    vec4 skinnedNormal = skinMatrix * vec4(vertexNormal_modelspace, 0.0);

    vertexPosition_worldspace = (modelMatrix * skinnedPosition).xyz;
    
    vec3 vertexPosition_cameraspace = (viewMatrix * modelMatrix * skinnedPosition).xyz;
    // Vector from vertex to camera
    eyeDirection_cameraspace = vec3(0, 0, 0) - vertexPosition_cameraspace;
    
    vec3 lightPosition_cameraspace = (viewMatrix * vec4(lightPosition_worldspace, 1)).xyz;
    lightDirection_cameraspace = lightPosition_cameraspace + eyeDirection_cameraspace; // Vector from vertex to light
    
    normal_cameraspace = (normalMatrix * modelMatrix * skinnedNormal).xyz;
    
    // Output position of the vertex
    gl_Position = MVP * skinnedPosition;

    materialId = vertexMaterialId;
}
