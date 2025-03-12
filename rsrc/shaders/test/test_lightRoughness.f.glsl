#version 330 core

in vec2 texcoord;
out vec3 color;

uniform sampler2D positionTex;
uniform sampler2D normalTex;
uniform sampler2D albedoTex;
uniform sampler2D metallicTex;
uniform sampler2D roughnessTex;

void main()
{
    vec3 fragPos_worldspace = texture(positionTex, texcoord).rgb;
	vec3 fragNormal_cameraspace = texture(normalTex, texcoord).rgb;
	vec3 albedo = texture(albedoTex, texcoord).rgb;
	float metallic = texture(metallicTex, texcoord).r;
	float roughness = texture(roughnessTex, texcoord).r;
    
    color = vec3(roughness);
}
