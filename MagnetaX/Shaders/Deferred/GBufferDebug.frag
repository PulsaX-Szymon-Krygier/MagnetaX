// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#version 450

layout(location = 0) in vec2 fragUV;

layout(set = 0, binding = 0) uniform sampler2D gAlbedo;
layout(set = 0, binding = 1) uniform sampler2D gNormal;
layout(set = 0, binding = 2) uniform sampler2D gMaterial;
layout(set = 0, binding = 3) uniform sampler2D gVelocity;
layout(set = 0, binding = 4) uniform sampler2D gDepth;

layout(push_constant) uniform PushConstants
{
    uint debugView;
    uint velocityAvailable;
} pc;

layout(location = 0) out vec4 outColor;

void main()
{
    if (pc.debugView == 0)
    {
        outColor = texture(gAlbedo, fragUV);
    }
    else if (pc.debugView == 1)
    {
        vec3 normal = texture(gNormal, fragUV).xyz;

        outColor = vec4(normal * 0.5 + 0.5, 1.0);
    }
    else if (pc.debugView == 2)
    {
        outColor = texture(gMaterial, fragUV);
    }
    else if (pc.debugView == 3)
    {
        if (pc.velocityAvailable == 0)
        {
            outColor = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }

        vec2 velocity = clamp(texture(gVelocity, fragUV).xy * 32.0, -1.0, 1.0);

        vec3 color = vec3(0.0);
        color.r = max(velocity.x, 0.0) + max(-velocity.y, 0.0);
        color.g = max(velocity.y, 0.0);
        color.b = max(-velocity.x, 0.0) + max(-velocity.y, 0.0);

        outColor = vec4(clamp(color, 0.0, 1.0), 1.0);
    }
    else if (pc.debugView == 4)
    {
        float depth = texture(gDepth, fragUV).r;

        outColor = vec4(depth, depth, depth, 1.0);
    }
}
