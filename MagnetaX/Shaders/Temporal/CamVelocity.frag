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

    if (depth < 0.999999)
    {
        discard;
    }

    vec2 currentNDC = fragUV * 2.0 - 1.0;

    vec4 currentClip = vec4(currentNDC, depth, 1.0);
    vec4 worldPosition = pc.invViewProj * currentClip;
    worldPosition /= worldPosition.w;

    vec4 prevClip = pc.prevViewProj * worldPosition;

    if (prevClip.w <= 0.0)
    {
        outVelocity = vec4(0.0);
        return;
    }

    vec3 prevNDC = prevClip.xyz / prevClip.w;

    vec2 velocityUV = 0.5 * (currentNDC - prevNDC.xy);
    float depthDelta = prevNDC.z - depth;

    outVelocity = vec4(velocityUV, depthDelta, 0.0);
}
