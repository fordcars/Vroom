#version 330 core

in vec2 texcoord;
out vec3 outColor;

uniform sampler2D colorTex;

vec3 fxaaTexture(sampler2D tex, vec2 coord) {
    vec2 texSize = 1.0 / textureSize(tex, 0); // Use inverse for easier multiplication
    float FXAA_SPAN_MAX = 8.0;
    float FXAA_REDUCE_MUL = 1.0 / 8.0;
    float FXAA_REDUCE_MIN = 1.0 / 128.0;

    // Sample surrounding pixels
    vec3 rgbNW = texture(tex, coord + (-vec2(1.0, 1.0) * texSize)).xyz;
    vec3 rgbNE = texture(tex, coord + (vec2(1.0, -1.0) * texSize)).xyz;
    vec3 rgbSW = texture(tex, coord + (-vec2(1.0, -1.0) * texSize)).xyz;
    vec3 rgbSE = texture(tex, coord + (vec2(1.0, 1.0) * texSize)).xyz;
    vec3 rgbM  = texture(tex, coord).xyz;

    // Luminance calculation
    vec3 lumaWeights = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, lumaWeights);
    float lumaNE = dot(rgbNE, lumaWeights);
    float lumaSW = dot(rgbSW, lumaWeights);
    float lumaSE = dot(rgbSE, lumaWeights);
    float lumaM  = dot(rgbM, lumaWeights);

    // Luminance min/max
    float lumaMin = min(min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)), lumaM);
    float lumaMax = max(max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)), lumaM);

    // Direction calculation
    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0 / (max(min(abs(dir.x), abs(dir.y)), dirReduce));

    // Normalize the direction
    dir = clamp(dir * rcpDirMin, -vec2(FXAA_SPAN_MAX), vec2(FXAA_SPAN_MAX)) * texSize;

    // Sample colors with the computed direction
    vec3 rgbA = 0.5 * (
        texture(tex, coord + dir * (1.0 / 3.0 - 0.5)).xyz +
        texture(tex, coord + dir * (2.0 / 3.0 - 0.5)).xyz
    );

    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(tex, coord + dir * (0.0 / 3.0 - 0.5)).xyz +
        texture(tex, coord + dir * (3.0 / 3.0 - 0.5)).xyz
    );

    float lumaB = dot(rgbB, lumaWeights);

    // Choose the final color based on luminance bounds
    return (lumaB < lumaMin || lumaB > lumaMax) ? rgbA : rgbB;
}

void main()
{
    outColor = fxaaTexture(colorTex, texcoord);
}
