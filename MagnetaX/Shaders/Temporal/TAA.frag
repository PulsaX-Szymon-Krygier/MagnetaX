// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#version 450

layout(location = 0) in vec2 fragUV;

layout(set = 0, binding = 0) uniform sampler2D currentColor;
layout(set = 0, binding = 1) uniform sampler2D historyColor;
layout(set = 0, binding = 2) uniform sampler2D velocityTexture;
layout(set = 0, binding = 3) uniform sampler2D depthTexture;

layout(push_constant) uniform PushConstants
{
    vec2 jitter;
    float feedbackMin;
    float feedbackMax;
    uint historyValid;
} pc;

layout(location = 0) out vec4 outColor;

float Luminance(vec3 color)
{
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

vec3 ClipHistory(vec3 history, vec3 boxMin, vec3 boxMax)
{
    const float epsilon = 0.000001;

    vec3 center = 0.5 * (boxMin + boxMax);
    vec3 extent = 0.5 * (boxMax - boxMin) + vec3(epsilon);
    vec3 offset = history - center;

    vec3 unitOffset = abs(offset / extent);
    float maxUnit = max(unitOffset.x, max(unitOffset.y, unitOffset.z));

    if (maxUnit > 1.0)
    {
        return center + offset / maxUnit;
    }

    return history;
}

void GetRoundedNeighborhood(vec2 uv, vec2 texelSize, out vec3 boxMin, out vec3 boxMax)
{
    vec3 topLeft = texture(currentColor, uv + vec2(-texelSize.x, -texelSize.y)).rgb;
    vec3 top = texture(currentColor, uv + vec2(0.0, -texelSize.y)).rgb;
    vec3 topRight = texture(currentColor, uv + vec2(texelSize.x, -texelSize.y)).rgb;

    vec3 left = texture(currentColor, uv + vec2(-texelSize.x, 0.0)).rgb;
    vec3 center = texture(currentColor, uv).rgb;
    vec3 right = texture(currentColor, uv + vec2(texelSize.x, 0.0)).rgb;

    vec3 bottomLeft = texture(currentColor, uv + vec2(-texelSize.x, texelSize.y)).rgb;
    vec3 bottom = texture(currentColor, uv + vec2(0.0, texelSize.y)).rgb;
    vec3 bottomRight = texture(currentColor, uv + vec2(texelSize.x, texelSize.y)).rgb;

    vec3 min9 = min(topLeft, min(top, min(topRight, min(left, min(center, min(right, min(bottomLeft, min(bottom, bottomRight))))))));
    vec3 max9 = max(topLeft, max(top, max(topRight, max(left, max(center, max(right, max(bottomLeft, max(bottom, bottomRight))))))));

    vec3 min5 = min(top, min(left, min(center, min(right, bottom))));
    vec3 max5 = max(top, max(left, max(center, max(right, bottom))));

    boxMin = 0.5 * (min9 + min5);
    boxMax = 0.5 * (max9 + max5);
}

vec2 GetClosestVelocity(vec2 uv, out float closestDepth)
{
    ivec2 imageSize = textureSize(depthTexture, 0);
    ivec2 maxCoord = imageSize - ivec2(1);
    ivec2 centerCoord = clamp(ivec2(uv * vec2(imageSize)), ivec2(0), maxCoord);

    ivec2 closestCoord = centerCoord;
    closestDepth = texelFetch(depthTexture, centerCoord, 0).r;

    for (int y = -1; y <= 1; ++y)
    {
        for (int x = -1; x <= 1; ++x)
        {
            ivec2 coord = clamp(centerCoord + ivec2(x, y), ivec2(0), maxCoord);
            float depth = texelFetch(depthTexture, coord, 0).r;

            if (depth < closestDepth)
            {
                closestDepth = depth;
                closestCoord = coord;
            }
        }
    }

    return texelFetch(velocityTexture, closestCoord, 0).xy;
}

void main()
{
    vec2 currentUV = fragUV + pc.jitter * 0.5;
    vec4 current = texture(currentColor, currentUV);

    if (pc.historyValid == 0)
    {
        outColor = current;
        return;
    }

    float closestDepth = 1.0;
    vec2 velocity = GetClosestVelocity(fragUV, closestDepth);

    if (closestDepth >= 0.999999)
    {
        outColor = current;
        return;
    }

    vec2 previousUV = fragUV - velocity * 0.5;

    if (any(lessThan(previousUV, vec2(0.0))) || any(greaterThan(previousUV, vec2(1.0))))
    {
        outColor = current;
        return;
    }

    vec2 texelSize = 1.0 / vec2(textureSize(currentColor, 0));

    vec3 neighborhoodMin;
    vec3 neighborhoodMax;
    GetRoundedNeighborhood(fragUV, texelSize, neighborhoodMin, neighborhoodMax);

    vec4 history = texture(historyColor, previousUV);
    history.rgb = ClipHistory(history.rgb, neighborhoodMin, neighborhoodMax);

    float currentLuminance = Luminance(current.rgb);
    float historyLuminance = Luminance(history.rgb);

    float luminanceDifference = abs(currentLuminance - historyLuminance) / max(currentLuminance, max(historyLuminance, 0.2));
    float similarity = clamp(1.0 - luminanceDifference, 0.0, 1.0);
    float feedback = mix(pc.feedbackMin, pc.feedbackMax, similarity * similarity);

    outColor = mix(current, history, feedback);
}
