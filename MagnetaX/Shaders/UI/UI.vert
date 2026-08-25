// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;

layout(push_constant) uniform PushConstants
{
    vec2 viewportSize;
} pc;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec4 fragColor;

void main()
{
    vec2 position;

    //position.x = (inPosition.x / pc.viewportSize.x) * 2.0 - 1.0;
    //position.y = 1.0 - (inPosition.y / pc.viewportSize.y) * 2.0;

    position.x = (inPosition.x / pc.viewportSize.x) * 2.0 - 1.0;
    position.y = (inPosition.y / pc.viewportSize.y) * 2.0 - 1.0;

    gl_Position = vec4(position, 0.0, 1.0);

    fragUV = inUV;
    fragColor = inColor;
}
