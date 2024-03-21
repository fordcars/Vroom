// Essentially a simplified version of tinyobj::material_t
#pragma once
#include <tiny_obj_loader.h>

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

    ObjMaterial(const tinyobj::material_t& mat) {
        ambient[0] = mat.ambient[0];
        ambient[1] = mat.ambient[1];
        ambient[2] = mat.ambient[2];

        diffuse[0] = mat.diffuse[0];
        diffuse[1] = mat.diffuse[1];
        diffuse[2] = mat.diffuse[2];

        specular[0] = mat.specular[0];
        specular[1] = mat.specular[1];
        specular[2] = mat.specular[2];

        transmittance[0] = mat.transmittance[0];
        transmittance[1] = mat.transmittance[1];
        transmittance[2] = mat.transmittance[2];

        emission[0] = mat.emission[0];
        emission[1] = mat.emission[1];
        emission[2] = mat.emission[2];

        shininess = mat.shininess;
        ior       = mat.ior;
        dissolve  = mat.dissolve;

        roughness           = mat.roughness;
        metallic            = mat.metallic;
        sheen               = mat.sheen;
        clearcoat_thickness = mat.clearcoat_thickness;
        clearcoat_roughness = mat.clearcoat_roughness;
        anisotropy          = mat.anisotropy;
        anisotropy_rotation = mat.anisotropy_rotation;
    }
};