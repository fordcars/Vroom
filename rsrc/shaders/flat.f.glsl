#version 330 core
#define MAX_MATERIALS 30

flat in int materialId;
in vec3 vertexPosition_worldspace;
in vec2 texcoord;

uniform sampler2D baseColorTex;
uniform int hasBaseColorTex;

out vec3 outColor;

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
    ObjMaterial mat = objMaterialsBlock.objMaterials[materialId];

    if (hasBaseColorTex == 1) {
        outColor = texture(baseColorTex, texcoord).rgb;
    } else {
        outColor = mat.baseColor;
    }
}