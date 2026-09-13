// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#version 450

layout(location = 0) in vec2 fragUV;

layout(set = 0, binding = 0) uniform sampler2D currentColor;
layout(set = 0, binding = 1) uniform sampler2D historyColor;
layout(set = 0, binding = 2) uniform sampler2D velocityTexture;
layout(set = 0, binding = 3) uniform sampler2D depthTexture;
layout(set = 0, binding = 4) uniform sampler2D previousDepthTexture;
layout(set = 0, binding = 5, r32ui) uniform readonly uimage2D reconstrPrevDepthImage;
layout(set = 0, binding = 6) uniform sampler2D previousSupportMean;
layout(set = 0, binding = 7) uniform sampler2D previousSupportSigma;

layout(set = 0, binding = 8) uniform TAAFrameData
{
    mat4 currentViewProj;
    mat4 previousInvViewProj;
} frameData;

layout(push_constant) uniform PushConstants
{
    vec2 jitterUV;
    vec2 prevJitterUV;
    float feedbackMin;
    float feedbackMax;
    uint historyValid;
    float nearPlane;
    float farPlane;
    vec2 projScale;
} pc;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outSupportMean;
layout(location = 2) out vec4 outSupportSigma;

float Luminance(vec3 color)
{
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

vec3 RGBToYCoCg(vec3 color)
{
    return vec3(dot(color, vec3(0.25, 0.5, 0.25)), dot(color, vec3(0.5, 0.0, -0.5)), dot(color, vec3(-0.25, 0.5, -0.25)));
}

vec3 YCoCgToRGB(vec3 color)
{
    return vec3(color.x + color.y - color.z, color.x + color.z, color.x - color.y - color.z);
}

void GetRawNeighborhood(vec2 uv, out vec3 mean, out vec3 stdDev)
{
    ivec2 imageSize = textureSize(currentColor, 0);
    ivec2 maxCoord = imageSize - ivec2(1);

    vec2 samplePosition = uv * vec2(imageSize) - vec2(0.5);
    ivec2 centerCoord = ivec2(floor(samplePosition + vec2(0.5)));

    vec3 moment1 = vec3(0.0);
    vec3 moment2 = vec3(0.0);
    float weightSum = 0.0;

    for (int y = -1; y <= 1; ++y)
    {
        for (int x = -1; x <= 1; ++x)
        {
            ivec2 rawCoord = centerCoord + ivec2(x, y);
            ivec2 coord = clamp(rawCoord, ivec2(0), maxCoord);

            vec2 delta = vec2(rawCoord) - samplePosition;
            vec2 weights = max(vec2(1.0) - abs(delta) / 1.5, vec2(0.0));
            float weight = weights.x * weights.y;

            if (weight <= 0.0) continue;

            vec3 sampleYCoCg = RGBToYCoCg(texelFetch(currentColor, coord, 0).rgb);

            moment1 += sampleYCoCg * weight;
            moment2 += sampleYCoCg * sampleYCoCg * weight;
            weightSum += weight;
        }
    }

    mean = moment1 / weightSum;

    vec3 variance = max(moment2 / weightSum - mean * mean, vec3(0.0));
    stdDev = sqrt(variance);
}

vec4 GetClosestVelocity(vec2 uv, out float closestDepth, out vec2 closestUV)
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

    closestUV = (vec2(closestCoord) + vec2(0.5)) / vec2(imageSize);

    return texelFetch(velocityTexture, closestCoord, 0);
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

    //return (0.5 * (a + b) * w0.x + a * s0.x + 0.5 * (a + b) * w3.x) * w0.y + (b * w0.x + c * s0.x + d * w3.x) *
    //    s0.y + (0.5 * (b + e) * w0.x + e * s0.x + 0.5 * (d + e) * w3.x) * w3.y;

    //return (0.5 * (a + b) * w0.x + a * s0.x + 0.5 * (a + d) * w3.x) * w0.y + (b * w0.x + c * s0.x + d * w3.x) * s0.y + 
    //    (0.5 * (b + e) * w0.x + e * s0.x + 0.5 * (d + e) * w3.x) * w3.y;

    float weightA = s0.x * w0.y;
    float weightB = w0.x * s0.y;
    float weightC = s0.x * s0.y;
    float weightD = w3.x * s0.y;
    float weightE = s0.x * w3.y;
    float weightSum = weightA + weightB + weightC + weightD + weightE;

    vec4 result = (a * weightA + b * weightB + c * weightC + d * weightD + e * weightE) / weightSum;

    vec3 minColor = min(min(a.rgb, b.rgb), min(c.rgb, min(d.rgb, e.rgb)));
    vec3 maxColor = max(max(a.rgb, b.rgb), max(c.rgb, max(d.rgb, e.rgb)));

    result.rgb = clamp(result.rgb, minColor, maxColor);

    return result;
}

float GetPreviousDepth(vec2 uv)
{
    vec4 depths = textureGather(previousDepthTexture, uv, 0);
    return max(max(depths.x, depths.y), max(depths.z, depths.w));
}

float LinearizeDepth(float depth)
{
    return (pc.nearPlane * pc.farPlane) / (pc.farPlane - depth * (pc.farPlane - pc.nearPlane));
}

float LoadReconstructedPrevDepth(ivec2 coord)
{
    return uintBitsToFloat(imageLoad(reconstrPrevDepthImage, coord).r);
}

float ComputeSurfaceValidity(vec2 uv, float expectedDepth, out float reconstructionCoverage)
{
    reconstructionCoverage = 0.0;

    if (any(lessThan(uv, vec2(0.0))) || any(greaterThanEqual(uv, vec2(1.0)))) return 0.0;

    ivec2 imageSize = imageSize(reconstrPrevDepthImage);

    vec2 st = uv * vec2(imageSize) - vec2(0.5);
    ivec2 baseCoord = ivec2(floor(st));
    vec2 fractional = fract(st);

    ivec2 offsets[4] = ivec2[4](ivec2(0, 0), ivec2(1, 0), ivec2(0, 1), ivec2(1, 1));
    float weights[4] = float[4]((1.0 - fractional.x) * (1.0 - fractional.y), fractional.x * (1.0 - fractional.y), (1.0 - fractional.x) * fractional.y, fractional.x * fractional.y);

    float expectedViewDepth = LinearizeDepth(expectedDepth);

    float tanHalfFovX = 1.0 / max(abs(pc.projScale.x), 0.000001);
    float tanHalfFovY = 1.0 / max(abs(pc.projScale.y), 0.000001);
    float fovScale = sqrt(1.0 + tanHalfFovX * tanHalfFovX + tanHalfFovY * tanHalfFovY);

    float resolutionLength = length(vec2(imageSize));
    float resolutionFactor = clamp(resolutionLength / length(vec2(1920.0, 1080.0)), 0.0, 1.0);
    float validityPower = mix(1.0, 3.0, resolutionFactor);

    float surfaceValidity = 0.0;

    for (int i = 0; i < 4; ++i)
    {
        ivec2 coord = baseCoord + offsets[i];

        if (any(lessThan(coord, ivec2(0))) || any(greaterThanEqual(coord, imageSize))) continue;

        float reconstructedDepth = LoadReconstructedPrevDepth(coord);

        if (reconstructedDepth >= 1.0) continue;

        reconstructionCoverage += weights[i];

        float reconstructedViewDepth = LinearizeDepth(reconstructedDepth);
        float depthSeparation = expectedViewDepth - reconstructedViewDepth;
        float sampleValidity = 1.0;

        if (depthSeparation > 0.0)
        {
            const float separationScale = 1.37e-05;

            float referenceDepth = max(expectedViewDepth, reconstructedViewDepth);
            float requiredSeparation = separationScale * fovScale * resolutionLength * referenceDepth;

            sampleValidity = pow(clamp(requiredSeparation / depthSeparation, 0.0, 1.0), validityPower);
        }

        surfaceValidity += sampleValidity * weights[i];
    }

    return surfaceValidity;
}

float ComputeRawLineEvidence(vec2 uv)
{
    ivec2 imageSize = textureSize(currentColor, 0);
    ivec2 maxCoord = imageSize - ivec2(1);

    vec2 samplePosition = uv * vec2(imageSize) - vec2(0.5);
    ivec2 centerCoord = clamp(ivec2(floor(samplePosition + vec2(0.5))), ivec2(0), maxCoord);

    float center = Luminance(texelFetch(currentColor, centerCoord, 0).rgb);

    const ivec2 directions[4] = ivec2[4](ivec2(1, 0), ivec2(0, 1), ivec2(1, 1), ivec2(1, -1));

    float lineEvidence = 0.0;

    for (int i = 0; i < 4; ++i)
    {
        ivec2 previousCoord = clamp(centerCoord - directions[i], ivec2(0), maxCoord);
        ivec2 nextCoord = clamp(centerCoord + directions[i], ivec2(0), maxCoord);

        float previous = Luminance(texelFetch(currentColor, previousCoord, 0).rgb);
        float next = Luminance(texelFetch(currentColor, nextCoord, 0).rgb);

        float previousDelta = center - previous;
        float nextDelta = center - next;

        if (previousDelta * nextDelta <= 0.0) continue;

        float previousContrast = abs(previousDelta) / max(max(center, previous), 0.05);
        float nextContrast = abs(nextDelta) / max(max(center, next), 0.05);

        lineEvidence = max(lineEvidence, min(previousContrast, nextContrast));
    }

    return clamp(lineEvidence, 0.0, 1.0);
}

const uint supportAgeMask = 31u;
const uint supportDriftMask = 31u;
const uint supportDriftShift = 5u;
const uint supportActiveBit = 1u << 10;
const float supportMaxDriftPixels = 0.5;

float ViewDepthToDeviceDepth(float viewDepth)
{
    return (pc.farPlane - pc.nearPlane * pc.farPlane / viewDepth) / (pc.farPlane - pc.nearPlane);
}

float ComputeGuideMotionError(vec2 guideUV, vec2 referenceVelocity, float previousGuideInverseDepth, out float currentGuideInverseDepth)
{
    currentGuideInverseDepth = 0.0;

    if (previousGuideInverseDepth <= 0.0) return supportMaxDriftPixels;

    ivec2 imageSize = textureSize(previousSupportMean, 0);

    float previousGuideViewDepth = 1.0 / previousGuideInverseDepth;
    float previousGuideDeviceDepth = ViewDepthToDeviceDepth(previousGuideViewDepth);

    vec2 previousNDC = guideUV * 2.0 - 1.0;
    vec4 worldPositionH = frameData.previousInvViewProj * vec4(previousNDC, previousGuideDeviceDepth, 1.0);

    if (abs(worldPositionH.w) < 0.000001) return supportMaxDriftPixels;

    vec3 worldPosition = worldPositionH.xyz / worldPositionH.w;
    vec4 currentClip = frameData.currentViewProj * vec4(worldPosition, 1.0);

    if (currentClip.w <= 0.0) return supportMaxDriftPixels;

    float currentDeviceDepth = currentClip.z / currentClip.w;

    if (currentDeviceDepth < 0.0 || currentDeviceDepth > 1.0) return supportMaxDriftPixels;

    float currentViewDepth = LinearizeDepth(currentDeviceDepth);

    if (currentViewDepth <= 0.0) return supportMaxDriftPixels;

    currentGuideInverseDepth = 1.0 / currentViewDepth;

    vec2 currentUV = (currentClip.xy / currentClip.w) * 0.5 + 0.5;
    vec2 guideVelocity = currentUV - guideUV;

    return length((guideVelocity - referenceVelocity) * vec2(imageSize));
}

float DecodeSupportDrift(uint state)
{
    uint quantizedDrift = (state >> supportDriftShift) & supportDriftMask;
    return float(quantizedDrift) * supportMaxDriftPixels / float(supportDriftMask);
}

void GetPreviousSupportMetadata(vec2 uv, out uint missingAge, out float driftPixels, out vec2 guideUV, out float guideInverseDepth, out float activeCoverage)
{
    ivec2 imageSize = textureSize(previousSupportSigma, 0);
    ivec2 maxCoord = imageSize - ivec2(1);

    vec2 st = uv * vec2(imageSize) - vec2(0.5);
    ivec2 baseCoord = ivec2(floor(st));
    vec2 fractional = fract(st);

    ivec2 offsets[4] = ivec2[4](ivec2(0, 0), ivec2(1, 0), ivec2(0, 1), ivec2(1, 1));
    float weights[4] = float[4]((1.0 - fractional.x) * (1.0 - fractional.y), fractional.x * (1.0 - fractional.y), 
        (1.0 - fractional.x) * fractional.y, fractional.x * fractional.y);

    missingAge = 0u;
    driftPixels = 0.0;
    guideInverseDepth = 0.0;
    activeCoverage = 0.0;
    guideUV = vec2(0.0);

    float bestGuideWeight = -1.0;

    for (int i = 0; i < 4; ++i)
    {
        if (weights[i] <= 0.0) continue;

        ivec2 coord = clamp(baseCoord + offsets[i], ivec2(0), maxCoord);

        vec4 sigmaState = texelFetch(previousSupportSigma, coord, 0);
        uint state = uint(round(sigmaState.a));

        if ((state & supportActiveBit) == 0u) continue;

        activeCoverage += weights[i];

        uint sampleAge = state & supportAgeMask;
        float sampleDrift = DecodeSupportDrift(state);

        missingAge = max(missingAge, sampleAge);
        driftPixels = max(driftPixels, sampleDrift);

        float sampleGuide = texelFetch(previousSupportMean, coord, 0).a;

        if (sampleGuide > 0.0 && weights[i] > bestGuideWeight)
        {
            bestGuideWeight = weights[i];
            guideUV = (vec2(coord) + vec2(0.5)) / vec2(imageSize);
            guideInverseDepth = sampleGuide;
        }
    }

    activeCoverage = clamp(activeCoverage, 0.0, 1.0);
}

float PackSupportState(uint missingAge, float driftPixels, bool isActive)
{
    uint quantizedDrift = uint(round(clamp(driftPixels / supportMaxDriftPixels, 0.0, 1.0) * float(supportDriftMask)));
    uint state = min(missingAge, supportAgeMask) | (quantizedDrift << supportDriftShift);

    if (isActive) state |= supportActiveBit;

    return float(state);
}

void GetPreviousSupportMoments(vec2 uv, out vec3 mean, out vec3 stdDev)
{
    ivec2 imageSize = textureSize(previousSupportMean, 0);
    ivec2 maxCoord = imageSize - ivec2(1);

    vec2 st = uv * vec2(imageSize) - vec2(0.5);
    ivec2 baseCoord = ivec2(floor(st));
    vec2 fractional = fract(st);

    ivec2 offsets[4] = ivec2[4](ivec2(0, 0), ivec2(1, 0), ivec2(0, 1), ivec2(1, 1));
    float weights[4] = float[4]((1.0 - fractional.x) * (1.0 - fractional.y), fractional.x * (1.0 - fractional.y), (1.0 - fractional.x) * fractional.y, fractional.x * fractional.y);

    vec3 sampleMeans[4];
    vec3 sampleStdDevs[4];

    mean = vec3(0.0);

    for (int i = 0; i < 4; ++i)
    {
        ivec2 coord = clamp(baseCoord + offsets[i], ivec2(0), maxCoord);

        sampleMeans[i] = texelFetch(previousSupportMean, coord, 0).rgb;
        sampleStdDevs[i] = texelFetch(previousSupportSigma, coord, 0).rgb;

        mean += sampleMeans[i] * weights[i];
    }

    vec3 variance = vec3(0.0);

    for (int i = 0; i < 4; ++i)
    {
        vec3 deviation = sampleMeans[i] - mean;
        variance += weights[i] * (sampleStdDevs[i] * sampleStdDevs[i] + deviation * deviation);
    }

    stdDev = sqrt(max(variance, vec3(0.0)));
}

void PoolTemporalMoments(vec3 currentMean, vec3 currentStdDev, vec3 previousMean, vec3 previousStdDev, out vec3 temporalMean, out vec3 temporalStdDev)
{
    const float eta = 1.0 / 16.0;

    vec3 currentVariance = currentStdDev * currentStdDev;
    vec3 previousVariance = previousStdDev * previousStdDev;
    vec3 meanDelta = currentMean - previousMean;

    temporalMean = mix(previousMean, currentMean, eta);

    vec3 temporalVariance = (1.0 - eta) * previousVariance + eta * currentVariance + eta * (1.0 - eta) * meanDelta * meanDelta;

    temporalStdDev = sqrt(max(temporalVariance, vec3(0.0)));
}

void main()
{
    vec2 currentUV = fragUV + pc.jitterUV;
    vec4 current = texture(currentColor, currentUV);
    current.a = 1.0;

    vec3 neighborhoodMean;
    vec3 neighborhoodStdDev;

    GetRawNeighborhood(currentUV, neighborhoodMean, neighborhoodStdDev);

    vec3 supportMeanValue = neighborhoodMean;
    vec3 supportStdDevValue = neighborhoodStdDev;

    outSupportMean = vec4(supportMeanValue, 0.0);
    outSupportSigma = vec4(supportStdDevValue, 0.0);

    if (pc.historyValid == 0)
    {
        outColor = current;
        return;
    }

    float closestDepth = 1.0;
    vec2 closestUV;
    vec4 velocity = GetClosestVelocity(currentUV, closestDepth, closestUV);

    if (velocity.w < 0.5)
    {
        outColor = current;
        return;
    }

    vec2 previousUV = fragUV - velocity.xy;
    float expectedPreviousDepth = closestDepth + velocity.z;

    if (any(lessThan(previousUV, vec2(0.0))) || any(greaterThanEqual(previousUV, vec2(1.0))))
    {
        outColor = current;
        return;
    }

    vec2 reconstructedUV = currentUV - pc.jitterUV - velocity.xy + pc.prevJitterUV;

    bool expectedDepthValid = expectedPreviousDepth >= 0.0 && expectedPreviousDepth <= 1.0;

    float reconstructionCoverage = 1.0;
    float surfaceValidity = 1.0;

    if (closestDepth < 1.0)
    {
        reconstructionCoverage = 0.0;

        if (expectedDepthValid)
        {
            surfaceValidity = ComputeSurfaceValidity(reconstructedUV, expectedPreviousDepth, reconstructionCoverage);
        }
        else
        {
            surfaceValidity = 0.0;
        }
    }

    bool additionalSupportValid = surfaceValidity >= 0.999 && reconstructionCoverage >= 0.999;
    //bool additionalSupportValid = true;

    float lineEvidence = ComputeRawLineEvidence(currentUV);
    bool currentObserved = lineEvidence > 0.0;

    uint previousMissingAge = 0u;
    float previousDrift = 0.0;
    vec2 previousGuideUV = vec2(0.0);
    float previousGuide = 0.0;
    float previousSupportCoverage = 0.0;

    if (additionalSupportValid)
    {
        GetPreviousSupportMetadata(previousUV, previousMissingAge, previousDrift, previousGuideUV, previousGuide, previousSupportCoverage);
    }

    bool previousSupportActive = previousSupportCoverage > 0.0;

    bool supportActive = false;
    bool reusePreviousSupport = false;
    uint missingAge = 0u;
    float supportGuide = 0.0;
    float supportDrift = 0.0;
    float supportCoverage = 0.0;

    if (additionalSupportValid)
    {
        if (currentObserved)
        {
            supportActive = true;
            supportCoverage = 1.0;

            if (closestDepth < 1.0)
            {
                supportGuide = 1.0 / max(LinearizeDepth(closestDepth), 0.000001);
            }

            if (previousSupportActive)
            {
                float currentPreviousGuide = 0.0;
                float guideMotionError = ComputeGuideMotionError(previousGuideUV, velocity.xy, previousGuide, currentPreviousGuide);

                reusePreviousSupport = guideMotionError < supportMaxDriftPixels;
            }
        }
        else if (previousSupportActive && previousMissingAge < 15u)
        {
            float currentPreviousGuide = 0.0;
            float guideMotionError = ComputeGuideMotionError(previousGuideUV, velocity.xy, previousGuide, currentPreviousGuide);
            float nextDrift = previousDrift + guideMotionError;

            if (nextDrift < supportMaxDriftPixels && currentPreviousGuide > 0.0)
            {
                supportActive = true;
                reusePreviousSupport = true;
                missingAge = previousMissingAge + 1u;
                supportGuide = currentPreviousGuide;
                supportDrift = nextDrift;
                supportCoverage = previousSupportCoverage;
            }
        }
    }

    if (reusePreviousSupport)
    {
        vec3 previousSupportMeanValue;
        vec3 previousSupportStdDev;

        GetPreviousSupportMoments(previousUV, previousSupportMeanValue, previousSupportStdDev);

        PoolTemporalMoments(neighborhoodMean, neighborhoodStdDev, previousSupportMeanValue, previousSupportStdDev, supportMeanValue, supportStdDevValue);
    }

    outSupportMean = vec4(supportMeanValue, supportGuide);
    outSupportSigma = vec4(supportStdDevValue, PackSupportState(missingAge, supportDrift, supportActive));

    vec4 history = SampleHistoryCatmullRom(previousUV);
    float previousHistoryMass = textureLod(historyColor, previousUV, 0.0).a;
    vec3 historyYCoCg = RGBToYCoCg(history.rgb);

    vec2 imageSize = vec2(textureSize(currentColor, 0));
    vec2 velocityPixels = velocity.xy * imageSize;

    float velocityConfidence = clamp(1.0 - length(velocityPixels) / 128.0, 0.0, 1.0);
    float varianceGamma = mix(0.75, 2.0, velocityConfidence * velocityConfidence);

    vec3 currentVarianceExtent = neighborhoodStdDev * varianceGamma;
    vec3 currentVarianceMin = neighborhoodMean - currentVarianceExtent;
    vec3 currentVarianceMax = neighborhoodMean + currentVarianceExtent;

    vec3 supportVarianceExtent = supportStdDevValue * varianceGamma;
    vec3 supportVarianceMin = supportMeanValue - supportVarianceExtent;
    vec3 supportVarianceMax = supportMeanValue + supportVarianceExtent;

    float supportAgeWeight = 1.0 - float(missingAge) / 16.0;
    float additionalSupportWeight = supportActive ? supportCoverage * supportAgeWeight * surfaceValidity : 0.0;
    //float additionalSupportWeight = supportActive ? 1.0 : 0.0;

    vec3 varianceMin = mix(currentVarianceMin, min(currentVarianceMin, supportVarianceMin), additionalSupportWeight);
    vec3 varianceMax = mix(currentVarianceMax, max(currentVarianceMax, supportVarianceMax), additionalSupportWeight);

    vec3 clippedHistoryYCoCg = clamp(historyYCoCg, varianceMin, varianceMax);

    vec3 historyDeviation = abs(historyYCoCg - neighborhoodMean);
    vec3 acceptedDeviation = abs(clippedHistoryYCoCg - neighborhoodMean);
    vec3 historyAcceptance = min(acceptedDeviation / max(historyDeviation, vec3(0.000001)), vec3(1.0));
    historyAcceptance = mix(vec3(1.0), historyAcceptance, greaterThan(historyDeviation, vec3(0.000001)));
    float acceptedHistory = min(historyAcceptance.x, min(historyAcceptance.y, historyAcceptance.z));

    historyYCoCg = clippedHistoryYCoCg;
    history.rgb = YCoCgToRGB(historyYCoCg);

    float currentLuminance = Luminance(current.rgb);
    float historyLuminance = Luminance(history.rgb);

    float luminanceDifference = abs(currentLuminance - historyLuminance) / max(currentLuminance, max(historyLuminance, 0.2));
    float similarity = clamp(1.0 - luminanceDifference, 0.0, 1.0);

    float feedback = mix(pc.feedbackMin, pc.feedbackMax, similarity * similarity);

    const float maxHistoryMass = 65504.0;
    float historyBudget = min(feedback / max(1.0 - feedback, 0.000001), maxHistoryMass);
    float acceptedHistoryMass = min(max(previousHistoryMass, 0.0), historyBudget);

    acceptedHistoryMass *= acceptedHistory;
    acceptedHistoryMass *= surfaceValidity;

    float currentWeight = 1.0 / (acceptedHistoryMass + 1.0);

    outColor.rgb = mix(history.rgb, current.rgb, currentWeight);
    outColor.a = min(acceptedHistoryMass + 1.0, historyBudget);
}
