#version 330 core
#define MAX_MATERIALS 30

flat in int materialId;
in vec3 normal_cameraspace;
in vec3 lightDirection_cameraspace;
in vec3 vertexPosition_worldspace;
in vec3 eyeDirection_cameraspace;
out vec3 outColor;

struct ObjMaterial {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 transmittance;
    vec3 emission;
    float shininess;
    float dissolve;  // 1 == opaque; 0 == fully transparent

    // PBR extension
    // http://exocortex.com/blog/extending_wavefront_mtl_to_support_pbr
    float roughness;            // [0, 1] default 0
    float metallic;             // [0, 1] default 0
    float sheen;                // [0, 1] default 0
    float p1;
    float p2;
    float p3;
};

layout(std140) uniform ObjMaterialsBlock {
    ObjMaterial objMaterials[MAX_MATERIALS];
} objMaterialsBlock;

vec2 blinnPhongDir(vec3 lightDir, float lightInt, float diffuseIntensity, float specularIntensity, float shininess)
{
    vec3 s = normalize(lightDir);
    vec3 v = normalize(-vertexPosition_worldspace);
    vec3 n = normalize(normal_cameraspace);
    vec3 h = normalize(v+s);
    
    float diffuse = diffuseIntensity * lightInt * max(0.0, dot(n, s));
    float specular = specularIntensity * lightInt * pow(max(0.0, dot(n, h)), shininess);
    
    return vec2(diffuse, specular);
}

void main()
{
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    float lightPower = 20.0;

    vec3 materialDiffuseColor = objMaterialsBlock.objMaterials[materialId].diffuse;
    vec3 materialAmbientColor = objMaterialsBlock.objMaterials[materialId].ambient * 0.1;
    vec3 materialSpecularColor = objMaterialsBlock.objMaterials[materialId].specular;
    
    float diffuseIntensity = 0.1;
    float specularIntensity = 0.1;
    float shininess = 10.0;
    vec2 lighting = blinnPhongDir(lightDirection_cameraspace, lightPower, diffuseIntensity, specularIntensity, shininess);
    
    outColor =
        materialAmbientColor +
        materialDiffuseColor * lightColor * lighting.x +
        materialSpecularColor * lightColor * lighting.y;
}