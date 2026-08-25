// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#version 450

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform PushConstants
{
    mat4 mvp;
} pc;

void main()
{
    gl_Position = pc.mvp * vec4(inPosition, 1.0);
}
