#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
uniform mat4 MVP;

out vec3 fScreenPos;

struct ObjMaterial {
    float ambient[3];
    float diffuse[3];
    float specular[3];
    float transmittance[3];
    float emission[3];
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

layout(shared) uniform ObjMaterialsBlock {
    ObjMaterial objMaterials[2];
};

void main()
{
    vec4 v = vec4(vertexPosition_modelspace, 1);
    gl_Position = MVP * v;
    
    fScreenPos = vec3(gl_Position.xyz);
}