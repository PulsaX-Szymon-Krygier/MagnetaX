// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec4 fragColor;

layout(set = 0, binding = 0) uniform sampler2D uiTexture;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = fragColor * texture(uiTexture, fragUV);
}
