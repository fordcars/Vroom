#version 330 core
#define MAX_MATERIALS 30

flat in int materialId;
in vec3 vertexPosition_worldspace;
in vec3 normal_cameraspace;

layout(location = 0) out vec3 fragPosition_worldspace;
layout(location = 1) out vec3 fragNormal_cameraspace;
layout(location = 2) out vec3 albedo;
layout(location = 3) out float metallic;
layout(location = 4) out float roughness;

struct ObjMaterial {
    vec3 baseColor;
    vec3 emission;

    float alpha;
    float roughness;
    float metallic;
    float sheen;

    int baseColorTextureIndex;
    int metallicRoughnessTextureIndex;
    int normalTextureIndex;
    int emissiveTextureIndex;
};

layout(std140) uniform ObjMaterialsBlock {
    ObjMaterial objMaterials[MAX_MATERIALS];
} objMaterialsBlock;

void main()
{
    fragPosition_worldspace = vertexPosition_worldspace;
    fragNormal_cameraspace = normalize(normal_cameraspace);
    albedo = objMaterialsBlock.objMaterials[materialId].baseColor;
    metallic = objMaterialsBlock.objMaterials[materialId].metallic;
    roughness = objMaterialsBlock.objMaterials[materialId].roughness;
}