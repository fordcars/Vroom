#version 330 core
#define MAX_MATERIALS 30

flat in int materialId;
in vec3 normal_cameraspace;
in vec3 lightDirection_cameraspace;
in vec3 vertexPosition_worldspace;
in vec3 eyeDirection_cameraspace;
out vec3 outColor;

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

vec2 blinnPhongDir(vec3 lightDir, float lightInt, float diffuseIntensity, float specularIntensity, float shininess)
{
    vec3 s = normalize(lightDir);
    vec3 v = normalize(eyeDirection_cameraspace);
    vec3 n = normalize(normal_cameraspace);
    vec3 h = normalize(v+s);
    
    float diffuse = diffuseIntensity * lightInt * max(0.0, dot(n, s));
    float specular = specularIntensity * lightInt * pow(max(0.0, dot(n, h)), shininess);
    
    return vec2(diffuse, specular);
}

void main()
{
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    float lightPower = 5.0;

    vec3 materialDiffuseColor = objMaterialsBlock.objMaterials[materialId].baseColor;
    vec3 materialAmbientColor = lightColor * 0.12;
    vec3 materialSpecularColor = vec3(1.0f, 1.0f, 1.0f) *
        objMaterialsBlock.objMaterials[materialId].metallic;
    
    float diffuseIntensity = 0.1;
    float specularIntensity = 0.1;
    float roughness = objMaterialsBlock.objMaterials[materialId].roughness;
    float shininess = exp2((1.0 - roughness) * 10.0);
    vec2 lighting = blinnPhongDir(lightDirection_cameraspace, lightPower, diffuseIntensity, specularIntensity, shininess);
    
    outColor =
        materialAmbientColor +
        materialDiffuseColor * lightColor * lighting.x +
        materialSpecularColor * lightColor * lighting.y;
}