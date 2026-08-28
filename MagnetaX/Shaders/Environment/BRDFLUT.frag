// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#version 450

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265;
const uint SAMPLE_COUNT = 512;

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

float GeometrySchlickGGX(float nDotDirection, float roughness)
{
    float a = roughness;
    float k = (a * a) / 2.0;

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

vec2 IntegrateBRDF(float nDotV, float roughness)
{
    vec3 normal = vec3(0.0, 0.0, 1.0);

    vec3 viewDirection;
    viewDirection.x = sqrt(max(1.0 - nDotV * nDotV, 0.0));
    viewDirection.y = 0.0;
    viewDirection.z = nDotV;

    float a = 0.0;
    float b = 0.0;

    for (uint i = 0; i < SAMPLE_COUNT; ++i)
    {
        vec2 xi = Hammersley(i, SAMPLE_COUNT);

        vec3 halfDirection = ImportanceSampleGGX(xi, normal, roughness);
        vec3 lightDirection = normalize(2.0 * dot(viewDirection, halfDirection) * halfDirection - viewDirection);

        float nDotL = max(lightDirection.z, 0.0);
        float nDotH = max(halfDirection.z, 0.0);
        float vDotH = max(dot(viewDirection, halfDirection), 0.0);

        if (nDotL > 0.0)
        {
            float geometry = GeometrySmith(normal, viewDirection, lightDirection, roughness);

            float geometryVisibility = (geometry * vDotH) / max(nDotH * nDotV, 0.0001);
            float fresnel = pow(1.0 - vDotH, 5.0);

            a += (1.0 - fresnel) * geometryVisibility;
            b += fresnel * geometryVisibility;
        }
    }

    return vec2(a, b) / float(SAMPLE_COUNT);
}

void main()
{
    float nDotV = fragUV.x;
    float roughness = fragUV.y;

    vec2 integratedBRDF = IntegrateBRDF(nDotV, roughness);

    outColor = vec4(integratedBRDF, 0.0, 1.0);
}
