// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#version 450

layout(location = 0) in vec2 fragUV;

layout(set = 0, binding = 0) uniform samplerCube environmentMap;

layout(push_constant) uniform PushConstants
{
    uint faceIndex;
    float roughness;
} pc;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265;
const uint SAMPLE_COUNT = 512;

vec3 GetDir(uint faceIndex, vec2 uv)
{
    vec2 pos = uv * 2.0 - 1.0;

    switch (faceIndex)
    {
    case 0:
        return normalize(vec3(1.0, -pos.y, -pos.x));
    case 1:
        return normalize(vec3(-1.0, -pos.y, pos.x));
    case 2:
        return normalize(vec3(pos.x, 1.0, pos.y));
    case 3:
        return normalize(vec3(pos.x, -1.0, -pos.y));
    case 4:
        return normalize(vec3(pos.x, -pos.y, 1.0));
    default:
        return normalize(vec3(-pos.x, -pos.y, -1.0));
    }
}

float RadicalInverse(uint bits)
{
    return float(bitfieldReverse(bits)) * 2.3283064365386963e-10;
}

vec2 Hammersley(uint index, uint count)
{
    return vec2(float(index) / float(count), RadicalInverse(index));
}

vec3 ImportanceSampleGGX(vec2 xi, vec3 normal, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;

    float phi = 2.0 * PI * xi.x;
    float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a2 - 1.0) * xi.y));
    float sinTheta = sqrt(max(1.0 - cosTheta * cosTheta, 0.0));

    vec3 halfDirection = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

    vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, normal));
    vec3 bitangent = cross(normal, tangent);

    return normalize(tangent * halfDirection.x + bitangent * halfDirection.y + normal * halfDirection.z);
}

float DistributionGGX(vec3 normal, vec3 halfDirection, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;

    float nDotH = max(dot(normal, halfDirection), 0.0);
    float nDotH2 = nDotH * nDotH;

    float denominator = nDotH2 * (a2 - 1.0) + 1.0;

    return a2 / max(PI * denominator * denominator, 0.0001);
}

void main()
{
    vec3 normal = GetDir(pc.faceIndex, fragUV);
    vec3 viewDirection = normal;

    vec3 color = vec3(0.0);
    float weight = 0.0;

    float resolution = float(textureSize(environmentMap, 0).x);
    float texelSolidAngle = 4.0 * PI / (6.0 * resolution * resolution);
    float maxMipLevel = float(textureQueryLevels(environmentMap) - 1);

    for (uint i = 0; i < SAMPLE_COUNT; ++i)
    {
        vec2 xi = Hammersley(i, SAMPLE_COUNT);
        vec3 halfDirection = ImportanceSampleGGX(xi, normal, pc.roughness);
        vec3 lightDirection = normalize(2.0 * dot(viewDirection, halfDirection) * halfDirection - viewDirection);

        float nDotL = max(dot(normal, lightDirection), 0.0);

        if (nDotL > 0.0)
        {
            float nDotH = max(dot(normal, halfDirection), 0.0);
            float hDotV = max(dot(halfDirection, viewDirection), 0.0);

            float distribution = DistributionGGX(normal, halfDirection, pc.roughness);
            float pdf = max((distribution * nDotH) / max(4.0 * hDotV, 0.0001), 0.0001);

            float sampleSolidAngle = 1.0 / (float(SAMPLE_COUNT) * pdf);

            float mipLevel = 0.0;

            if (pc.roughness > 0.0)
            {
                mipLevel = 0.5 * log2(sampleSolidAngle / texelSolidAngle);
            }

            mipLevel = clamp(mipLevel, 0.0, maxMipLevel);

            color += textureLod(environmentMap, lightDirection, mipLevel).rgb * nDotL;
            weight += nDotL;
        }
    }

    color /= max(weight, 0.0001);

    outColor = vec4(color, 1.0);
}
