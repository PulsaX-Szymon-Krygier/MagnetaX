// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#version 450

layout(location = 0) in vec2 fragUV;

layout(set = 0, binding = 1) uniform sampler2D gNormal;
layout(set = 1, binding = 0) uniform samplerCube environmentMap;

layout(push_constant) uniform PushConstants
{
    mat4 viewProjectionInversed;
    vec4 cameraPosition;
} pc;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 normalValue = texture(gNormal, fragUV).xyz;

    if (dot(normalValue, normalValue) >= 0.0001)
    {
        discard;
    }

    vec4 clipPosition = vec4(fragUV * 2.0 - 1.0, 1.0, 1.0);

    vec4 worldPositionValue = pc.viewProjectionInversed * clipPosition;
    vec3 worldPosition = worldPositionValue.xyz / worldPositionValue.w;

    vec3 direction = normalize(worldPosition - pc.cameraPosition.xyz);

    outColor = texture(environmentMap, direction);
}
