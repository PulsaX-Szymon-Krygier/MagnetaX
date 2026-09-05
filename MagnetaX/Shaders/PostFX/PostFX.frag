// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#version 450

layout(location = 0) in vec2 fragUV;

layout(set = 0, binding = 0) uniform sampler2D ldrColor;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants
{
    uint fxaaEnabled;
    float contrastThreshold;
    float relativeThreshold;
    float subpixelBlending;
} pc;

// FXAA 3.11 shader code
struct LuminanceData
{
    float m;
    float n;
    float e;
    float s;
    float w;

    float ne;
    float nw;
    float se;
    float sw;

    float highest;
    float lowest;
    float contrast;
};

struct EdgeData
{
    bool isHorizontal;
    float pixelStep;
    float oppositeLuminance;
    float gradient;
};

vec4 Sample(vec2 uv)
{
    return texture(ldrColor, uv);
}

float SampleLuminance(vec2 uv)
{
    return Sample(uv).a;
}

float SampleLuminance(vec2 uv, float uOffset, float vOffset, vec2 texelSize)
{
    return SampleLuminance(uv + vec2(uOffset, vOffset) * texelSize);
}

LuminanceData SampleLuminanceNeighborhood(vec2 uv, vec2 texelSize)
{
    LuminanceData l;

    l.m = SampleLuminance(uv);
    l.n = SampleLuminance(uv, 0.0, -1.0, texelSize);
    l.e = SampleLuminance(uv, 1.0, 0.0, texelSize);
    l.s = SampleLuminance(uv, 0.0, 1.0, texelSize);
    l.w = SampleLuminance(uv, -1.0, 0.0, texelSize);

    l.ne = SampleLuminance(uv, 1.0, -1.0, texelSize);
    l.nw = SampleLuminance(uv, -1.0, -1.0, texelSize);
    l.se = SampleLuminance(uv, 1.0, 1.0, texelSize);
    l.sw = SampleLuminance(uv, -1.0, 1.0, texelSize);

    l.highest = max(l.m, max(max(l.n, l.s), max(l.e, l.w)));
    l.lowest = min(l.m, min(min(l.n, l.s), min(l.e, l.w)));
    l.contrast = l.highest - l.lowest;

    return l;
}

bool ShouldSkipPixel(LuminanceData l)
{
    float threshold = max(pc.contrastThreshold, pc.relativeThreshold * l.highest);

    return l.contrast < threshold;
}

float DeterminePixelBlendFactor(LuminanceData l)
{
    float filteredLuminance = 2.0 * (l.n + l.e + l.s + l.w);
    filteredLuminance += l.ne + l.nw + l.se + l.sw;
    filteredLuminance *= 1.0 / 12.0;

    filteredLuminance = abs(filteredLuminance - l.m);
    filteredLuminance = clamp(filteredLuminance / l.contrast, 0.0, 1.0);

    float blendFactor = smoothstep(0.0, 1.0, filteredLuminance);

    return blendFactor * blendFactor * pc.subpixelBlending;
}

EdgeData DetermineEdge(LuminanceData l, vec2 texelSize)
{
    EdgeData edge;

    float horizontal = 2.0 * abs(l.n + l.s - 2.0 * l.m) +
        abs(l.ne + l.se - 2.0 * l.e) + abs(l.nw + l.sw - 2.0 * l.w);

    float vertical = 2.0 * abs(l.e + l.w - 2.0 * l.m) + 
        abs(l.ne + l.nw - 2.0 * l.n) + abs(l.se + l.sw - 2.0 * l.s);

    edge.isHorizontal = horizontal >= vertical;

    float positiveLuminance = edge.isHorizontal ? l.s : l.e;
    float negativeLuminance = edge.isHorizontal ? l.n : l.w;

    float positiveGradient = abs(positiveLuminance - l.m);
    float negativeGradient = abs(negativeLuminance - l.m);

    edge.pixelStep = edge.isHorizontal ? texelSize.y : texelSize.x;

    if (positiveGradient < negativeGradient)
    {
        edge.pixelStep = -edge.pixelStep;
        edge.oppositeLuminance = negativeLuminance;
        edge.gradient = negativeGradient;
    }
    else
    {
        edge.oppositeLuminance = positiveLuminance;
        edge.gradient = positiveGradient;
    }

    return edge;
}

float DetermineEdgeBlendFactor(LuminanceData l, EdgeData edge, vec2 uv, vec2 texelSize)
{
    vec2 edgeUV = uv;
    vec2 edgeStep;

    if (edge.isHorizontal)
    {
        edgeUV.y += edge.pixelStep * 0.5;
        edgeStep = vec2(texelSize.x, 0.0);
    }
    else
    {
        edgeUV.x += edge.pixelStep * 0.5;
        edgeStep = vec2(0.0, texelSize.y);
    }

    float edgeLuminance = 0.5 * (l.m + edge.oppositeLuminance);
    float gradientThreshold = edge.gradient * 0.25;

    vec2 positiveUV = edgeUV + edgeStep;
    vec2 negativeUV = edgeUV - edgeStep;

    float positiveLuminanceDelta = SampleLuminance(positiveUV) - edgeLuminance;
    float negativeLuminanceDelta = SampleLuminance(negativeUV) - edgeLuminance;

    bool positiveAtEnd = abs(positiveLuminanceDelta) >= gradientThreshold;
    bool negativeAtEnd = abs(negativeLuminanceDelta) >= gradientThreshold;

    const int edgeStepCount = 9;
    const float edgeSteps[edgeStepCount] = float[](1.5, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 4.0);

    for (int i = 0; i < edgeStepCount && (!positiveAtEnd || !negativeAtEnd); ++i)
    {
        if (!positiveAtEnd)
        {
            positiveUV += edgeStep * edgeSteps[i];
            positiveLuminanceDelta = SampleLuminance(positiveUV) - edgeLuminance;
            positiveAtEnd = abs(positiveLuminanceDelta) >= gradientThreshold;
        }

        if (!negativeAtEnd)
        {
            negativeUV -= edgeStep * edgeSteps[i];
            negativeLuminanceDelta = SampleLuminance(negativeUV) - edgeLuminance;
            negativeAtEnd = abs(negativeLuminanceDelta) >= gradientThreshold;
        }
    }

    const float edgeGuess = 8.0;

    if (!positiveAtEnd)
    {
        positiveUV += edgeStep * edgeGuess;
    }

    if (!negativeAtEnd)
    {
        negativeUV -= edgeStep * edgeGuess;
    }

    float positiveDistance;
    float negativeDistance;

    if (edge.isHorizontal)
    {
        positiveDistance = positiveUV.x - uv.x;
        negativeDistance = uv.x - negativeUV.x;
    }
    else
    {
        positiveDistance = positiveUV.y - uv.y;
        negativeDistance = uv.y - negativeUV.y;
    }

    float shortestDistance;
    float nearestLuminanceDelta;

    if (positiveDistance <= negativeDistance)
    {
        shortestDistance = positiveDistance;
        nearestLuminanceDelta = positiveLuminanceDelta;
    }
    else
    {
        shortestDistance = negativeDistance;
        nearestLuminanceDelta = negativeLuminanceDelta;
    }

    bool nearestDeltaSign = nearestLuminanceDelta >= 0.0;
    bool centerDeltaSign = (l.m - edgeLuminance) >= 0.0;

    if (nearestDeltaSign == centerDeltaSign)
    {
        return 0.0;
    }

    float edgeLength = positiveDistance + negativeDistance;

    return 0.5 - shortestDistance / edgeLength;
}

vec3 ApplyFXAA(vec2 uv)
{
    if (pc.fxaaEnabled == 0)
    {
        return Sample(uv).rgb;
    }

    vec2 texelSize = 1.0 / vec2(textureSize(ldrColor, 0));

    LuminanceData l = SampleLuminanceNeighborhood(uv, texelSize);

    if (ShouldSkipPixel(l))
    {
        //return vec3(0.0);
        return Sample(uv).rgb;
    }

    //return vec3(1.0, 0.0, 0.0);

    float pixelBlend = DeterminePixelBlendFactor(l);

    EdgeData edge = DetermineEdge(l, texelSize);
    float edgeBlend = DetermineEdgeBlendFactor(l, edge, uv, texelSize);

    float finalBlend = max(pixelBlend, edgeBlend);

    if (edge.isHorizontal)
    {
        uv.y += edge.pixelStep * finalBlend;
    }
    else
    {
        uv.x += edge.pixelStep * finalBlend;
    }

    return Sample(uv).rgb;
}
// End of FXAA

void main()
{
    outColor = vec4(ApplyFXAA(fragUV), 1.0);
}
