// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(set = 1, binding = 0) uniform FrameData
{
    mat4 viewProj;
    mat4 jitteredViewProj;
    mat4 prevViewProj;
} frameData;

layout(push_constant) uniform PushConstants
{
    mat4 model;
    mat4 prevModel;
} pc;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec4 fragCurrentStableClip;
layout(location = 3) out vec4 fragPrevStableClip;

void main()
{
    vec4 localPosition = vec4(inPosition, 1.0);

    vec4 worldPosition = pc.model * localPosition;
    vec4 prevWorldPosition = pc.prevModel * localPosition;

    fragCurrentStableClip = frameData.viewProj * worldPosition;
    fragPrevStableClip = frameData.prevViewProj * prevWorldPosition;

    gl_Position = frameData.jitteredViewProj * worldPosition;

    mat3 normalMatrix = transpose(inverse(mat3(pc.model)));
    fragNormal = normalize(normalMatrix * inNormal);

    fragUV = inUV;
}
