#version 330 core
#define MAX_MATERIALS 30

flat in int materialId;
in vec3 vertexPosition_worldspace;
in vec3 normal_cameraspace;
in vec2 texcoord;
in mat3 tbn;

uniform sampler2D baseColorTex;
uniform int hasBaseColorTex;
uniform sampler2D normalTex;
uniform int hasNormalTex;
uniform sampler2D metallicRoughnessTex;
uniform int hasMetallicRoughnessTex;
uniform sampler2D emissionTex;
uniform int hasEmissionTex;
uniform float normalScale;

layout(location = 0) out vec3 fragPosition_worldspace;
layout(location = 1) out vec3 fragNormal_cameraspace;
layout(location = 2) out vec3 albedo;
layout(location = 3) out float metallic;
layout(location = 4) out float roughness;

// std140-compatible struct
struct ObjMaterial {
    vec3 baseColor;
    float p1;
    vec3 emission;
    float p2;
    
    float alpha;
    float metallic;
    float roughness;
    float sheen;
};

layout(std140) uniform ObjMaterialsBlock {
    ObjMaterial objMaterials[MAX_MATERIALS];
} objMaterialsBlock;

void main()
{
    fragPosition_worldspace = vertexPosition_worldspace;
    ObjMaterial mat = objMaterialsBlock.objMaterials[materialId];

    if (hasBaseColorTex == 1) {
        albedo = texture(baseColorTex, texcoord).rgb;
    } else {
        albedo = mat.baseColor;
    }

    if (hasNormalTex == 1) {
        // Hack to transform tangent space normal to world space!
        vec3 normalMap = texture(normalTex, texcoord).rgb;
        normalMap = normalize((normalMap * 2.0 - 1.0)
            * vec3(normalScale, normalScale, 1.0));

        // Transform the normal from tangent space to world space
        fragNormal_cameraspace = normalize(tbn * normalMap);
    } else {
        fragNormal_cameraspace = normalize(normal_cameraspace);
    }

    if (hasMetallicRoughnessTex == 1) {
        vec2 mrSample = texture(metallicRoughnessTex, texcoord).bg; // (Metallic in B, Roughness in G)
        metallic = mrSample.r;
        roughness = mrSample.g;
    } else {
        metallic = mat.metallic;
        roughness = mat.roughness;
    }
}
