#version 330 core
#define MAX_MATERIALS 30

flat in int materialId;
in vec3 vertexPosition_worldspace;
in vec3 normal_cameraspace;
in vec2 texcoord;

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
// Why padding is necessary here? Not too sure,
// but it's definitely needed for the struct to work.
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

        // Construct the TBN matrix
        vec3 tangent = vec3(1.0, 0.0, 0.0); // Assume tangent aligned with x-axis
        vec3 bitangent = normalize(cross(normal_cameraspace, tangent)); // Compute bitangent
        
        // Normalize to ensure orthonormality
        if (length(bitangent) == 0.0) {
            bitangent = vec3(0.0, 1.0, 0.0); // Use an arbitrary direction if zero
        }
        tangent = normalize(cross(bitangent, normal_cameraspace)); // Recompute tangent based on bitangent

        // Construct the TBN matrix
        mat3 TBN = mat3(tangent, bitangent, normal_cameraspace);

        // Transform the normal from tangent space to world space
        fragNormal_cameraspace = normalize(TBN * normalMap);
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
