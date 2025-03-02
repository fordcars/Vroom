#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 2) in int vertexMaterialId;

// New attributes for skinning
layout(location = 3) in ivec4 boneIDs;  // Bone indices
layout(location = 4) in vec4 weights;   // Bone weights

// Matrices
uniform mat4 MVP;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 normalMatrix;

// Uniform Block for Bone Matrices
layout(std140) uniform BoneTransformsBlock {
    mat4 boneTransforms[100];  // MAX_BONES = 100
};

flat out int materialId;
out vec3 normal_cameraspace;
out vec3 lightDirection_cameraspace;
out vec3 vertexPosition_worldspace;
out vec3 eyeDirection_cameraspace;

void main()
{
    vec3 lightPosition_worldspace = vec3(60, 60, 40);

    // Apply skeletal transformation using weighted bone matrices
    mat4 skinMatrix = 
          weights.x * boneTransforms[boneIDs.x] +
          weights.y * boneTransforms[boneIDs.y] +
          weights.z * boneTransforms[boneIDs.z] +
          weights.w * boneTransforms[boneIDs.w];

    vec4 skinnedPosition = skinMatrix * vec4(vertexPosition_modelspace, 1.0);
    vec4 skinnedNormal = skinMatrix * vec4(vertexNormal_modelspace, 0.0);

    // Transform position to world space
    vertexPosition_worldspace = (modelMatrix * skinnedPosition).xyz;

    vec3 vertexPosition_cameraspace = (viewMatrix * modelMatrix * skinnedPosition).xyz;
    eyeDirection_cameraspace = -vertexPosition_cameraspace;  // Vector from vertex to camera

    vec3 lightPosition_cameraspace = (viewMatrix * vec4(lightPosition_worldspace, 1)).xyz;
    lightDirection_cameraspace = lightPosition_cameraspace + eyeDirection_cameraspace; // Vector from vertex to light

    normal_cameraspace = (normalMatrix * modelMatrix * skinnedNormal).xyz;

    // Apply final transformation
    gl_Position = MVP * skinnedPosition;

    materialId = vertexMaterialId;
}
