// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#version 450

layout(location = 0) in vec2 fragUV;

// GBuffer targets
layout(set = 0, binding = 0) uniform sampler2D gAlbedo;
layout(set = 0, binding = 1) uniform sampler2D gNormal;
layout(set = 0, binding = 2) uniform sampler2D gMaterial;
layout(set = 0, binding = 3) uniform sampler2D gDepth;

// Frame data
layout(set = 1, binding = 0) uniform FrameData
{
    mat4 view;
    mat4 viewProjectionInversed;
    vec4 cameraPosition;
    mat4 directionalShadowViewProjections[4];
    vec4 directionalShadowSplits;
    vec4 directionalShadowBiases;
    vec4 directionalShadowBlendWidths;
    vec4 ambientColorIntensity;
    vec4 lightInfo;
    vec4 clearColor;
    mat4 spotShadowViewProj;
} frameData;

// Light data
struct LightData
{
    vec4 positionRange;
    vec4 direction;
    vec4 colorIntensity;
    vec4 params;
};

// Light buffer
layout(std430, set = 1, binding = 1) readonly buffer LightBuffer
{
    LightData lights[];
} lightBuffer;

// Shadow map sampler
layout(set = 1, binding = 2) uniform sampler2DArrayShadow directionalShadowMap;
layout(set = 1, binding = 3) uniform sampler2DShadow spotShadowMap;
layout(set = 1, binding = 4) uniform samplerCube specularEnvironmentMap;
layout(set = 1, binding = 5) uniform sampler2D brdfLUT;

layout(location = 0) out vec4 outColor;

const uint LIGHT_DIRECTIONAL = 0;
const uint LIGHT_POINT = 1;
const uint LIGHT_SPOT = 2;

const float PI = 3.14159265;

float DistributionGGX(vec3 normal, vec3 halfDirection, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;

    float nDotH = max(dot(normal, halfDirection), 0.0);
    float nDotH2 = nDotH * nDotH;

    float denominator = nDotH2 * (a2 - 1.0) + 1.0;

    return a2 / (PI * denominator * denominator);
}

float GeometrySchlickGGX(float nDotDirection, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    return nDotDirection / (nDotDirection * (1.0 - k) + k);
}

float GeometrySmith(vec3 normal, vec3 viewDirection, vec3 lightDirection, float roughness)
{
    float nDotV = max(dot(normal, viewDirection), 0.0);
    float nDotL = max(dot(normal, lightDirection), 0.0);

    float geometryView = GeometrySchlickGGX(nDotV, roughness);
    float geometryLight = GeometrySchlickGGX(nDotL, roughness);

    return geometryView * geometryLight;
}

vec3 FresnelSchlick(float cosTheta, vec3 f0)
{
    return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 f0, float roughness)
{
    return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - cosTheta, 5.0);
}

vec3 CalculateEnvironmentSpecular(vec3 albedo, vec3 normal, vec3 viewDirection, float roughness, float metallic, float ambientOcclusion)
{
    //float envR = max(roughness, 0.25);

    vec3 f0 = mix(vec3(0.04), albedo, metallic);

    float nDotV = max(dot(normal, viewDirection), 0.0);
    vec3 fresnel = FresnelSchlickRoughness(nDotV, f0, roughness);
    //vec3 fresnel = FresnelSchlickRoughness(nDotV, f0, envR);

    vec3 reflectionDirection = reflect(-viewDirection, normal);

    float maxLod = float(textureQueryLevels(specularEnvironmentMap) - 1);
    //vec3 prefilteredColor = textureLod(specularEnvironmentMap, reflectionDirection, envR * maxLod).rgb;
    vec3 prefilteredColor = textureLod(specularEnvironmentMap, reflectionDirection, roughness * maxLod).rgb;

    //vec2 brdf = texture(brdfLUT, vec2(nDotV, envR)).rg;
    vec2 brdf = texture(brdfLUT, vec2(nDotV, roughness)).rg;

    vec3 specular = prefilteredColor * (fresnel * brdf.x + brdf.y);

    return specular * ambientOcclusion;
}

vec3 CalculatePBRBRDF(vec3 albedo, vec3 normal, vec3 viewDirection, vec3 lightDirection, float roughness, float metallic)
{
    float nDotL = max(dot(normal, lightDirection), 0.0);
    float nDotV = max(dot(normal, viewDirection), 0.0);

    if (nDotL <= 0.0 || nDotV <= 0.0) return vec3(0.0);

    vec3 halfDir = normalize(lightDirection + viewDirection);

    float distribution = DistributionGGX(normal, halfDir, roughness);
    float geometry = GeometrySmith(normal, viewDirection, lightDirection, roughness);

    vec3 f0 = mix(vec3(0.04), albedo, metallic);
    float hDotV = max(dot(halfDir, viewDirection), 0.0);
    vec3 fresnel = FresnelSchlick(hDotV, f0);

    vec3 numerator = distribution * geometry * fresnel;
    float denominator = max(4.0 * nDotV * nDotL, 0.0001);

    vec3 specular = numerator / denominator;

    vec3 kS = fresnel;
    vec3 kD = vec3(1.0) - kS;

    kD *= 1.0 - metallic;

    vec3 diffuse = kD * albedo / PI;

    return diffuse + specular;
    //return diffuse;
}

float CalculateDistanceAttenuation(float distanceToLight, float range)
{
    float distanceSquared = max(distanceToLight * distanceToLight, 0.01);

    float rangeFactor = clamp(1.0 - distanceToLight / range, 0.0, 1.0);
    float rangeAttenuation = rangeFactor * rangeFactor;

    return rangeAttenuation / distanceSquared;
}

vec3 CalculateDirectionalLight(LightData light, vec3 albedo, vec3 normal, vec3 viewDirection, float roughness, float metallic)
{
    vec3 lightDirection = normalize(-light.direction.xyz);

    float nDotL = max(dot(normal, lightDirection), 0.0);
    vec3 radiance = light.colorIntensity.rgb * light.colorIntensity.a;

    vec3 brdf = CalculatePBRBRDF(albedo, normal, viewDirection, lightDirection, roughness, metallic);

    return brdf * radiance * nDotL;
}

vec3 CalculatePointLight(LightData light, vec3 albedo, vec3 normal, vec3 worldPosition, vec3 viewDirection, float roughness, float metallic)
{
    vec3 toLight = light.positionRange.xyz - worldPosition;
    float distanceToLight = length(toLight);
    float range = light.positionRange.w;

    if (distanceToLight >= range || range <= 0.0) return vec3(0.0);

    vec3 lightDirection = toLight / max(distanceToLight, 0.0001);

    float nDotL = max(dot(normal, lightDirection), 0.0);
    float attenuation = CalculateDistanceAttenuation(distanceToLight, range);

    vec3 radiance = light.colorIntensity.rgb * light.colorIntensity.a * attenuation;
    vec3 brdf = CalculatePBRBRDF(albedo, normal, viewDirection, lightDirection, roughness, metallic);

    return brdf * radiance * nDotL;
}

vec3 CalculateSpotLight(LightData light, vec3 albedo, vec3 normal, vec3 worldPosition, vec3 viewDirection, float roughness, float metallic)
{
    vec3 lightToPixel = worldPosition - light.positionRange.xyz;
    float distanceToLight = length(lightToPixel);
    float range = light.positionRange.w;

    if (distanceToLight >= range || range <= 0.0) return vec3(0.0);

    vec3 lightToPixelDirection = lightToPixel / max(distanceToLight, 0.0001);
    vec3 spotDirection = normalize(light.direction.xyz);

    float theta = dot(lightToPixelDirection, spotDirection);
    float innerCos = cos(radians(light.params.x));
    float outerCos = cos(radians(light.params.y));
    float coneFactor = smoothstep(outerCos, innerCos, theta);

    if (coneFactor <= 0.0) return vec3(0.0);

    vec3 pixelToLightDirection = -lightToPixelDirection;

    float nDotL = max(dot(normal, pixelToLightDirection), 0.0);
    float attenuation = CalculateDistanceAttenuation(distanceToLight, range);

    vec3 radiance = light.colorIntensity.rgb * light.colorIntensity.a * attenuation * coneFactor;
    vec3 brdf = CalculatePBRBRDF(albedo, normal, viewDirection, pixelToLightDirection, roughness, metallic);

    return brdf * radiance * nDotL;
}

float CalculateDirectionalShadow(vec3 worldPosition, vec3 normal, vec3 lightDirection, int cascadeIndex)
{
    vec4 shadowClipPosition = frameData.directionalShadowViewProjections[cascadeIndex] * vec4(worldPosition, 1.0);
    vec3 shadowPosition = shadowClipPosition.xyz / shadowClipPosition.w;

    vec2 shadowUV = shadowPosition.xy * 0.5 + 0.5;

    if (shadowUV.x < 0.0 || shadowUV.x > 1.0 || shadowUV.y < 0.0 || shadowUV.y > 1.0) return 1.0;
    if (shadowPosition.z < 0.0 || shadowPosition.z > 1.0) return 1.0;

    float nDotL = max(dot(normal, lightDirection), 0.0);

    float sinTheta = sqrt(max(1.0 - nDotL * nDotL, 0.0));
    float slope = sinTheta / max(nDotL, 0.1);

    float baseBias = frameData.directionalShadowBiases[cascadeIndex];
    float biasScale = clamp(1.0 + slope * 2.0, 1.0, 4.0);
    float bias = baseBias * biasScale;

    vec2 texelSize = 1.0 / vec2(textureSize(directionalShadowMap, 0).xy);
    float referenceDepth = shadowPosition.z - bias;

    float visibility = 0.0;

    int quality = 2;

    for (int y = -quality; y <= quality; ++y)
    {
        for (int x = -quality; x <= quality; ++x)
        {
            visibility += texture(directionalShadowMap, vec4(shadowUV + vec2(x, y) * texelSize, float(cascadeIndex), referenceDepth));
        }
    }

    int diameter = quality * 2 + 1;
    return visibility / float(diameter * diameter);
}

float CalculateSpotShadow(LightData light, vec3 worldPosition, vec3 normal)
{
    vec4 shadowClipPosition = frameData.spotShadowViewProj * vec4(worldPosition, 1.0);
    vec3 shadowPosition = shadowClipPosition.xyz / shadowClipPosition.w;

    vec2 shadowUV = shadowPosition.xy * 0.5 + 0.5;

    if (shadowUV.x < 0.0 || shadowUV.x > 1.0 || shadowUV.y < 0.0 || shadowUV.y > 1.0) return 1.0;
    if (shadowPosition.z < 0.0 || shadowPosition.z > 1.0) return 1.0;

    vec3 toLight = light.positionRange.xyz - worldPosition;
    vec3 lightDirection = normalize(toLight);

    float nDotL = max(dot(normal, lightDirection), 0.0);
    float angleFactor = 1.0 - nDotL;

    const float biasMin = 0.0002;
    const float biasMax = 0.001;

    float bias = mix(biasMin, biasMax, angleFactor);

    return texture(spotShadowMap, vec3(shadowUV, shadowPosition.z - bias));
}

void main()
{
    vec3 albedo = texture(gAlbedo, fragUV).rgb;
    vec3 material = texture(gMaterial, fragUV).rgb;

    float roughness = clamp(material.r, 0.04, 1.0);
    //float roughness = clamp(material.r, 0.15, 1.0);
    //float roughness = 1.0;
    float metallic = clamp(material.g, 0.0, 1.0);
    float ambientOcclusion = clamp(material.b, 0.0, 1.0);

    vec3 normalValue = texture(gNormal, fragUV).xyz;

    if (dot(normalValue, normalValue) < 0.0001)
    {
        outColor = frameData.clearColor;
        return;
    }

    vec3 normal = normalize(normalValue);

    float depth = texture(gDepth, fragUV).r;

    //float filteredRoughness = sqrt(sqrt(filteredAlpha2));
    //float filteredRoughness = roughness;
    //float filteredRoughness = normalVariance > 0.001 ? 1.0 : roughness;

    //float depth = texture(gDepth, fragUV).r;
    vec4 clipPosition = vec4(fragUV * 2.0 - 1.0, depth, 1.0);
    vec4 worldPositionValue = frameData.viewProjectionInversed * clipPosition;
    vec3 worldPosition = worldPositionValue.xyz / worldPositionValue.w;
    vec3 viewDirection = normalize(frameData.cameraPosition.xyz - worldPosition);

    vec4 viewPosition = frameData.view * vec4(worldPosition, 1.0);
    float viewDepth = -viewPosition.z;

    int cascadeIndex = 0;

    if (viewDepth > frameData.directionalShadowSplits.x) cascadeIndex = 1;
    if (viewDepth > frameData.directionalShadowSplits.y) cascadeIndex = 2;
    if (viewDepth > frameData.directionalShadowSplits.z) cascadeIndex = 3;

    vec3 f0 = mix(vec3(0.04), albedo, metallic);
    vec3 ambientKD = (vec3(1.0) - f0) * (1.0 - metallic);

    vec3 ambient = ambientKD * albedo * frameData.ambientColorIntensity.rgb * frameData.ambientColorIntensity.a * ambientOcclusion;
    vec3 color = ambient;

    color += CalculateEnvironmentSpecular(albedo, normal, viewDirection, roughness, metallic, ambientOcclusion);
    //color += CalculateEnvironmentSpecular(albedo, normal, viewDirection, filteredRoughness, metallic, ambientOcclusion);

    uint lightCount = uint(frameData.lightInfo.x);

    for (uint i = 0; i < lightCount; ++i)
    {
        LightData light = lightBuffer.lights[i];
        uint lightType = uint(light.params.z);

        if (lightType == LIGHT_DIRECTIONAL)
        {
            vec3 lightDirection = normalize(-light.direction.xyz);
            vec3 directionalColor = CalculateDirectionalLight(light, albedo, normal, viewDirection, roughness, metallic);
            //vec3 directionalColor = CalculateDirectionalLight(light, albedo, normal, viewDirection, filteredRoughness, metallic);

            if (light.params.w > 0.5 && viewDepth <= frameData.directionalShadowSplits.w)
            {
                float shadow = CalculateDirectionalShadow(worldPosition, normal, lightDirection, cascadeIndex);

                if (cascadeIndex > 0)
                {
                    int previousCascadeIndex = cascadeIndex - 1;

                    float transitionStart = frameData.directionalShadowSplits[previousCascadeIndex];
                    float transitionWidth = frameData.directionalShadowBlendWidths[previousCascadeIndex];

                    if (transitionWidth > 0.0 && viewDepth < transitionStart + transitionWidth)
                    {
                        float blend = clamp((viewDepth - transitionStart) / transitionWidth, 0.0, 1.0);
                        float previousShadow = CalculateDirectionalShadow(worldPosition, normal, lightDirection, previousCascadeIndex);

                        shadow = mix(previousShadow, shadow, blend);
                    }
                }

                directionalColor *= shadow;
            }

            color += directionalColor;
        }
        else if (lightType == LIGHT_POINT)
        {
            color += CalculatePointLight(light, albedo, normal, worldPosition, viewDirection, roughness, metallic);
            //color += CalculatePointLight(light, albedo, normal, worldPosition, viewDirection, filteredRoughness, metallic);
        }
        else if (lightType == LIGHT_SPOT)
        {
            vec3 spotColor = CalculateSpotLight(light, albedo, normal, worldPosition, viewDirection, roughness, metallic);
            //vec3 spotColor = CalculateSpotLight(light, albedo, normal, worldPosition, viewDirection, filteredRoughness, metallic);

            if (light.params.w > 0.5) spotColor *= CalculateSpotShadow(light, worldPosition, normal);

            color += spotColor;
        }
    }

    outColor = vec4(color, 1.0);
}
