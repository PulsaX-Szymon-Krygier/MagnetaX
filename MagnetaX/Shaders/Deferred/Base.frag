// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragUV;

layout(set = 0, binding = 0) uniform sampler2D baseColorTexture;

layout(set = 0, binding = 1) uniform MaterialData
{
    vec4 baseColor;
    vec4 properties;
    vec2 uvScale;
} material;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outMaterial;

void main()
{
    vec4 baseColor = texture(baseColorTexture, fragUV * material.uvScale) * material.baseColor;

    vec3 normal = normalize(fragNormal);

    float roughness = material.properties.x;
    float metallic = material.properties.y;
    float ambientOcclusion = material.properties.z;

    outAlbedo = baseColor;
    outNormal = vec4(normal, 1.0);
    outMaterial = vec4(roughness, metallic, ambientOcclusion, 0.0);
}
