// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#version 450

layout(location = 0) in vec2 fragUV;

layout(set = 0, binding = 0) uniform sampler2D currentColor;
layout(set = 0, binding = 1) uniform sampler2D historyColor;
layout(set = 0, binding = 2) uniform sampler2D depthTexture;

layout(push_constant) uniform PushConstants
{
    mat4 reprojection;
    vec2 projectionJitter;
    float historyWeight;
    uint historyValid;
} pc;

layout(location = 0) out vec4 outColor;

void main()
{
    vec4 current = texture(currentColor, fragUV);
    //outColor = current;
    //return;

    if (pc.historyValid == 0)
    {
        outColor = current;
        return;
    }

    float depth = texture(depthTexture, fragUV).r;

    if (depth >= 0.999999)
    {
        outColor = current;
        return;
    }

    vec2 currentNDC = fragUV * 2.0 - 1.0;

    vec4 currentClip = vec4(currentNDC, depth, 1.0);
    vec4 previousClip = pc.reprojection * currentClip;

    if (previousClip.w <= 0.0)
    {
        outColor = current;
        return;
    }

    vec2 currentStableNDC = currentNDC - pc.projectionJitter;
    vec2 previousNDC = previousClip.xy / previousClip.w;

    vec2 previousUV = fragUV + (previousNDC - currentStableNDC) * 0.5;

    if (any(lessThan(previousUV, vec2(0.0))) || any(greaterThan(previousUV, vec2(1.0))))
    {
        outColor = current;
        return;
    }

    vec2 texelSize = 1.0 / vec2(textureSize(currentColor, 0));

    vec2 neighborhoodUV = fragUV - pc.projectionJitter * 0.5;

    vec3 mean = vec3(0.0);
    vec3 meanSquared = vec3(0.0);

    for (int y = -1; y <= 1; ++y)
    {
        for (int x = -1; x <= 1; ++x)
        {
            //vec3 sampleColor = texture(currentColor, fragUV + vec2(x, y) * texelSize).rgb;
            vec3 sampleColor = texture(currentColor, neighborhoodUV + vec2(x, y) * texelSize).rgb;

            mean += sampleColor;
            meanSquared += sampleColor * sampleColor;
        }
    }

    mean /= 9.0;
    meanSquared /= 9.0;

    vec3 variance = max(meanSquared - mean * mean, vec3(0.0));
    vec3 deviation = sqrt(variance);

    const float varianceGamma = 1.5;

    vec3 neighborhoodMin = mean - deviation * varianceGamma;
    vec3 neighborhoodMax = mean + deviation * varianceGamma;

    vec4 history = texture(historyColor, previousUV);
    history.rgb = clamp(history.rgb, neighborhoodMin, neighborhoodMax);

    outColor = mix(current, history, pc.historyWeight);
}
