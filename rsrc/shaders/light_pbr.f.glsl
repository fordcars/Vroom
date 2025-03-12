#version 330 core

in vec3 lightPos_cameraspace;
in vec2 texcoord;
out vec3 outColor;

uniform mat4 viewMatrix;

uniform vec3 lightDiffuseColor;
uniform vec3 lightSpecularColor;
uniform float lightIntensity;

uniform sampler2D positionTex;
uniform sampler2D normalTex;
uniform sampler2D albedoTex;
uniform sampler2D metallicTex;
uniform sampler2D roughnessTex;

vec2 blinnPhongDir(vec3 eyeDirCameraspace, vec3 normalCameraspace, 
                    vec3 lightDir, float lightInt, float diffuseIntensity,
                    float specularIntensity, float shininess)
{
    vec3 s = normalize(lightDir);
    vec3 v = normalize(eyeDirCameraspace);
    vec3 n = normalize(normalCameraspace);
    vec3 h = normalize(v + s);
    
    float diffuse = diffuseIntensity * lightInt * max(0.0, dot(n, s));
    float specular = specularIntensity * lightInt * pow(max(0.0, dot(n, h)), shininess);
    
    return vec2(diffuse, specular);
}

void main()
{
    vec3 fragPos_worldspace = texture(positionTex, texcoord).rgb;
    vec3 fragNormal_cameraspace = texture(normalTex, texcoord).rgb;
    vec3 albedo = texture(albedoTex, texcoord).rgb;
    float metallic = texture(metallicTex, texcoord).r;
    float roughness = texture(roughnessTex, texcoord).r;

    vec3 materialSpecularColor = vec3(1.0f, 1.0f, 1.0f) * metallic;

    vec3 fragPos_cameraspace = (viewMatrix * vec4(fragPos_worldspace, 1)).xyz;
    vec3 eyeDirection_cameraspace = vec3(0, 0, 0) - fragPos_cameraspace; // Frag to camera
    vec3 lightDirection_cameraspace = lightPos_cameraspace - fragPos_cameraspace; // Frag to light
    
    float diffuseIntensity = mix(0.25, 0.1, metallic); // Fully metallic surfaces have little diffuse
    float specularIntensity = 1.0 - roughness;
    float shininess = exp2((1.0 - roughness) * 10.0);
    
    vec2 lighting = blinnPhongDir(eyeDirection_cameraspace, fragNormal_cameraspace,
                                  lightDirection_cameraspace, lightIntensity, 
                                  diffuseIntensity, specularIntensity, shininess);
    
    outColor =
        albedo * 0.12 + // Ambient
        albedo * lightDiffuseColor * lighting.x +
        materialSpecularColor * lightSpecularColor * lighting.y;
}
