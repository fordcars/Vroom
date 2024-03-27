#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
uniform mat4 MVP;
out vec3 fScreenPos;

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
    ObjMaterial objMaterials[];
} objMaterialsBlock;

void main()
{
    vec4 v = vec4(vertexPosition_modelspace, 1);
    gl_Position = MVP * v;
    
    fScreenPos = vec3(gl_Position.xyz) * objMaterialsBlock.objMaterials[0].ambient;
}