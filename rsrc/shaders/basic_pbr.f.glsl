#version 330 core
#define MAX_MATERIALS 50

uniform vec3 color;
flat in int materialId;
out vec3 outColor;

struct ObjMaterial {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 transmittance;
    vec3 emission;
    float shininess;
    float ior;       // index of refraction
    float dissolve;  // 1 == opaque; 0 == fully transparent

    // PBR extension
    // http://exocortex.com/blog/extending_wavefront_mtl_to_support_pbr
    float roughness;            // [0, 1] default 0
    float metallic;             // [0, 1] default 0
    float sheen;                // [0, 1] default 0
    float clearcoat_thickness;  // [0, 1] default 0
    float clearcoat_roughness;  // [0, 1] default 0
    float anisotropy;           // aniso. [0, 1] default 0
    float anisotropy_rotation;  // anisor. [0, 1] default 0
};

layout(std140) uniform ObjMaterialsBlock {
    ObjMaterial objMaterials[MAX_MATERIALS];
} objMaterialsBlock;

void main()
{
    if(materialId == -1) {
        outColor = vec3(0.1, 0.1, 0.1);
    } else {
        vec3 finalColor = objMaterialsBlock.objMaterials[materialId].ambient
            + objMaterialsBlock.objMaterials[materialId].diffuse;
        outColor = clamp(finalColor, 0.0, 1.0);
    }
}