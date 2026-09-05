// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#version 450

layout(location = 0) in vec2 fragUV;

layout(set = 0, binding = 0) uniform sampler2D currentColor;
layout(set = 0, binding = 1) uniform sampler2D historyColor;
layout(set = 0, binding = 2) uniform sampler2D velocityTexture;
layout(set = 0, binding = 3) uniform sampler2D depthTexture;
layout(set = 0, binding = 4) uniform sampler2D previousDepthTexture;

layout(push_constant) uniform PushConstants
{
    vec2 jitterUV;
    vec2 prevJitterUV;
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

vec3 RGBToYCoCg(vec3 color)
{
    return vec3(dot(color, vec3(0.25, 0.5, 0.25)), dot(color, vec3(0.5, 0.0, -0.5)),
        dot(color, vec3(-0.25, 0.5, -0.25)));
}

vec3 YCoCgToRGB(vec3 color)
{
    return vec3(color.x + color.y - color.z, color.x + color.z, color.x - color.y - color.z);
}

void GetRoundedNeighborhood(vec2 uv, vec2 texelSize, out vec3 boxMin, out vec3 boxMax, out vec3 mean, out vec3 stdDev)
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

    vec3 topLeftYCoCg = RGBToYCoCg(topLeft);
    vec3 topYCoCg = RGBToYCoCg(top);
    vec3 topRightYCoCg = RGBToYCoCg(topRight);
    vec3 leftYCoCg = RGBToYCoCg(left);
    vec3 centerYCoCg = RGBToYCoCg(center);
    vec3 rightYCoCg = RGBToYCoCg(right);
    vec3 bottomLeftYCoCg = RGBToYCoCg(bottomLeft);
    vec3 bottomYCoCg = RGBToYCoCg(bottom);
    vec3 bottomRightYCoCg = RGBToYCoCg(bottomRight);

    vec3 moment1 = topLeftYCoCg + topYCoCg + topRightYCoCg + leftYCoCg + centerYCoCg + rightYCoCg +
        bottomLeftYCoCg + bottomYCoCg + bottomRightYCoCg;

    vec3 moment2 = topLeftYCoCg * topLeftYCoCg + topYCoCg * topYCoCg + topRightYCoCg * topRightYCoCg + 
        leftYCoCg * leftYCoCg + centerYCoCg * centerYCoCg + rightYCoCg * rightYCoCg + bottomLeftYCoCg * 
        bottomLeftYCoCg + bottomYCoCg * bottomYCoCg + bottomRightYCoCg * bottomRightYCoCg;

    mean = moment1 / 9.0;

    vec3 variance = max(moment2 / 9.0 - mean * mean, vec3(0.0));
    stdDev = sqrt(variance);

    vec3 min9 = min(topLeft, min(top, min(topRight, min(left, min(center, min(right, min(bottomLeft, min(bottom, bottomRight))))))));
    vec3 max9 = max(topLeft, max(top, max(topRight, max(left, max(center, max(right, max(bottomLeft, max(bottom, bottomRight))))))));

    vec3 min5 = min(top, min(left, min(center, min(right, bottom))));
    vec3 max5 = max(top, max(left, max(center, max(right, bottom))));

    boxMin = 0.5 * (min9 + min5);
    boxMax = 0.5 * (max9 + max5);
}

vec3 GetClosestVelocity(vec2 uv, out float closestDepth)
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

    return texelFetch(velocityTexture, closestCoord, 0).xyz;
}

vec4 SampleHistoryCatmullRom(vec2 uv)
{
    vec2 imageSize = vec2(textureSize(historyColor, 0));
    vec2 rcpResolution = 1.0 / imageSize;
    vec2 historyST = uv * imageSize - vec2(0.5);
    vec2 fractional = fract(historyST);
    vec2 baseUV = (floor(historyST) + vec2(0.5)) * rcpResolution;

    vec2 t = fractional;
    vec2 t2 = t * t;
    vec2 t3 = t2 * t;

    const float s = 0.5;

    vec2 w0 = -s * t3 + 2.0 * s * t2 - s * t;
    vec2 w1 = (2.0 - s) * t3 + (s - 3.0) * t2 + vec2(1.0);
    vec2 w2 = (s - 2.0) * t3 + (3.0 - 2.0 * s) * t2 + s * t;
    vec2 w3 = s * t3 - s * t2;

    vec2 s0 = w1 + w2;
    vec2 f0 = w2 / s0;

    vec2 m0 = baseUV + f0 * rcpResolution;
    vec2 tc0 = baseUV - rcpResolution;
    vec2 tc3 = baseUV + 2.0 * rcpResolution;

    vec4 a = texture(historyColor, vec2(m0.x, tc0.y));
    vec4 b = texture(historyColor, vec2(tc0.x, m0.y));
    vec4 c = texture(historyColor, m0);
    vec4 d = texture(historyColor, vec2(tc3.x, m0.y));
    vec4 e = texture(historyColor, vec2(m0.x, tc3.y));

    return (0.5 * (a + b) * w0.x + a * s0.x + 0.5 * (a + b) * w3.x) * w0.y + (b * w0.x + c * s0.x + d * w3.x) *
        s0.y + (0.5 * (b + e) * w0.x + e * s0.x + 0.5 * (d + e) * w3.x) * w3.y;
}

float GetPreviousDepth(vec2 uv)
{
    vec4 depths = textureGather(previousDepthTexture, uv, 0);
    return max(max(depths.x, depths.y), max(depths.z, depths.w));
}

vec3 ClipToAABB(vec3 history, vec3 current, vec3 center, vec3 extent)
{
    vec3 direction = current - history;
    vec3 intersection = ((center - sign(direction) * extent) - history) / direction;

    const float maxT = 10000.0;

    vec3 possibleT = vec3(maxT + 1.0);
    if (intersection.x >= 0.0) possibleT.x = intersection.x;
    if (intersection.y >= 0.0) possibleT.y = intersection.y;
    if (intersection.z >= 0.0) possibleT.z = intersection.z;

    float t = min(maxT, min(possibleT.x, min(possibleT.y, possibleT.z)));

    return t < maxT ? history + direction * t : history;
}

void main()
{
    vec2 currentUV = fragUV + pc.jitterUV;
    vec4 current = texture(currentColor, currentUV);

    if (pc.historyValid == 0)
    {
        //outColor = vec4(current.rgb, 0.5);
        outColor = current;
        return;
    }

    float closestDepth = 1.0;
    vec3 velocity = GetClosestVelocity(fragUV, closestDepth);

    vec2 previousUV = fragUV - velocity.xy;
    float expectedPreviousDepth = closestDepth + velocity.z;

    if (any(lessThan(previousUV, vec2(0.0))) || any(greaterThan(previousUV, vec2(1.0))))
    {
        outColor = current;
        return;
    }

    vec2 previousDepthUV = previousUV + pc.prevJitterUV;

    if (any(lessThan(previousDepthUV, vec2(0.0))) || any(greaterThanEqual(previousDepthUV, vec2(1.0))))
    {
        outColor = current;
        return;
    }

    float previousDepth = GetPreviousDepth(previousDepthUV);

    const float depthBias = 0.001;

    if (expectedPreviousDepth > previousDepth + depthBias)
    {
        outColor = current;
        return;
    }

    vec2 texelSize = 1.0 / vec2(textureSize(currentColor, 0));

    vec3 neighborhoodMin;
    vec3 neighborhoodMax;
    vec3 neighborhoodMean;
    vec3 neighborhoodStdDev;

    GetRoundedNeighborhood(fragUV, texelSize, neighborhoodMin, neighborhoodMax, neighborhoodMean, neighborhoodStdDev);
    //GetRoundedNeighborhood(currentUV, texelSize, neighborhoodMin, neighborhoodMax, neighborh oodMean, neighborhoodStdDev);

    vec4 history = SampleHistoryCatmullRom(previousUV);

    vec2 imageSize = vec2(textureSize(currentColor, 0));
    vec2 velocityPixels = velocity.xy * imageSize;

    float velocityConfidence = clamp(1.0 - length(velocityPixels) / 128.0, 0.0, 1.0);
    float varianceGamma = mix(0.75, 2.0, velocityConfidence * velocityConfidence);

    vec3 varianceExtent = neighborhoodStdDev * varianceGamma;
    vec3 varianceMin = YCoCgToRGB(neighborhoodMean - varianceExtent);
    vec3 varianceMax = YCoCgToRGB(neighborhoodMean + varianceExtent);

    vec3 historyYCoCg = RGBToYCoCg(history.rgb);
    vec3 currentYCoCg = RGBToYCoCg(current.rgb);

    history.rgb = clamp(history.rgb, varianceMin, varianceMax);
    //history.rgb = YCoCgToRGB(ClipToAABB(historyYCoCg, currentYCoCg, neighborhoodMean, varianceExtent));

    //outColor = history;
    //return;

    float currentLuminance = Luminance(current.rgb);
    float historyLuminance = Luminance(history.rgb);

    float luminanceDifference = abs(currentLuminance - historyLuminance) / max(currentLuminance, max(historyLuminance, 0.2));
    float similarity = clamp(1.0 - luminanceDifference, 0.0, 1.0);
    float feedback = mix(pc.feedbackMin, pc.feedbackMax, similarity * similarity);

    outColor = mix(current, history, feedback);
}
