// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#version 450

layout(location = 0) in vec2 fragUV;

layout(set = 0, binding = 0) uniform sampler2D depthTexture;

layout(push_constant) uniform PushConstants
{
    mat4 invViewProj;
    mat4 prevViewProj;
} pc;

layout(location = 0) out vec4 outVelocity;

void main()
{
    ivec2 imageSize = textureSize(depthTexture, 0);
    ivec2 maxCoord = imageSize - ivec2(1);
    ivec2 coord = clamp(ivec2(fragUV * vec2(imageSize)), ivec2(0), maxCoord);

    float depth = texelFetch(depthTexture, coord, 0).r;

    if (depth < 1.0)
    {
        discard;
    }

    vec2 currentNDC = fragUV * 2.0 - 1.0;

    vec4 currentNearH = pc.invViewProj * vec4(currentNDC, 0.0, 1.0);
    vec4 currentFarH = pc.invViewProj * vec4(currentNDC, 1.0, 1.0);

    vec3 currentNear = currentNearH.xyz / currentNearH.w;
    vec3 currentFar = currentFarH.xyz / currentFarH.w;
    vec3 worldDirection = normalize(currentFar - currentNear);

    vec4 prevClip = pc.prevViewProj * vec4(worldDirection, 0.0);

    if (prevClip.w <= 0.0)
    {
        outVelocity = vec4(0.0);
        return;
    }

    vec2 prevNDC = prevClip.xy / prevClip.w;
    vec2 velocityUV = 0.5 * (currentNDC - prevNDC);

    outVelocity = vec4(velocityUV, 0.0, 1.0);
}
