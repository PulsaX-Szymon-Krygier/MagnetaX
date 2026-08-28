// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#version 450

layout(location = 0) in vec2 fragUV;

layout(set = 0, binding = 0) uniform sampler2D equirectMap;

layout(push_constant) uniform PushConstants
{
    uint faceIndex;
} pc;

layout(location = 0) out vec4 outColor;

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

vec2 GetEquirectUV(vec3 direction)
{
    const float invTwoPi = 0.15915494;
    const float invPi = 0.31830989;

    vec2 uv;
    uv.x = atan(direction.z, direction.x) * invTwoPi + 0.5;
    uv.y = 0.5 - asin(clamp(direction.y, -1.0, 1.0)) * invPi;

    return uv;
}

void main()
{
    vec3 direction = GetDir(pc.faceIndex, fragUV);
    vec2 uv = GetEquirectUV(direction);

    //outColor = texture(equirectMap, uv);

    vec4 color = texture(equirectMap, uv);
    color.rgb = min(color.rgb, vec3(65504.0));

    outColor = color;
}
