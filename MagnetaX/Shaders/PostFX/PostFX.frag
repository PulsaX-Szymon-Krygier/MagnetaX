// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#version 450

layout(location = 0) in vec2 fragUV;

layout(set = 0, binding = 0) uniform sampler2D sceneColor;

layout(location = 0) out vec4 outColor;

// ITU-R BT.601 standard (Luma formula)
// Red (Kr) = 0.299
// Green (Kg) = 0.587
// Blue (Kb) = 0.114
float Luma(vec3 color)
{
    return dot(color, vec3(0.299, 0.587, 0.114));
}

vec3 ToneMapACES(vec3 color)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

vec3 ProcessColor(vec3 color)
{
    const float exposureEV = -3.0;

    color *= exp2(exposureEV);

    return ToneMapACES(color);
}

void main()
{
    const float edgeThresholdMin = 0.03;
    const float edgeThreshold = 0.125;
    const float subpixelQuality = 0.75;
    const int searchSteps = 12;

    vec2 texelSize = 1.0 / vec2(textureSize(sceneColor, 0));

    //vec3 colorCenter = texture(sceneColor, fragUV).rgb;
    vec3 colorCenter = ProcessColor(texture(sceneColor, fragUV).rgb);

    float lumaCenter = Luma(colorCenter);
    float lumaNorth = Luma(ProcessColor(texture(sceneColor, fragUV + vec2(0.0, -texelSize.y)).rgb));
    float lumaSouth = Luma(ProcessColor(texture(sceneColor, fragUV + vec2(0.0, texelSize.y)).rgb));
    float lumaWest = Luma(ProcessColor(texture(sceneColor, fragUV + vec2(-texelSize.x, 0.0)).rgb));
    float lumaEast = Luma(ProcessColor(texture(sceneColor, fragUV + vec2(texelSize.x, 0.0)).rgb));

    float lumaNorthWest = Luma(ProcessColor(texture(sceneColor, fragUV + vec2(-texelSize.x, -texelSize.y)).rgb));
    float lumaNorthEast = Luma(ProcessColor(texture(sceneColor, fragUV + vec2(texelSize.x, -texelSize.y)).rgb));
    float lumaSouthWest = Luma(ProcessColor(texture(sceneColor, fragUV + vec2(-texelSize.x, texelSize.y)).rgb));
    float lumaSouthEast = Luma(ProcessColor(texture(sceneColor, fragUV + vec2(texelSize.x, texelSize.y)).rgb));

    float lumaMin = min(lumaCenter, min(min(lumaNorth, lumaSouth), min(lumaWest, lumaEast)));
    float lumaMax = max(lumaCenter, max(max(lumaNorth, lumaSouth), max(lumaWest, lumaEast)));

    float contrast = lumaMax - lumaMin;
    float threshold = max(edgeThresholdMin, lumaMax * edgeThreshold);

    if (contrast < threshold)
    {
        outColor = vec4(colorCenter, 1.0);
        return;
    }

    float edgeHorizontal = abs(lumaNorthWest + lumaSouthWest - 2.0 * lumaWest) + 2.0 * abs(lumaNorth + lumaSouth - 2.0 * lumaCenter) + abs(lumaNorthEast + lumaSouthEast - 2.0 * lumaEast);
    float edgeVertical = abs(lumaNorthWest + lumaNorthEast - 2.0 * lumaNorth) + 2.0 * abs(lumaWest + lumaEast - 2.0 * lumaCenter) + abs(lumaSouthWest + lumaSouthEast - 2.0 * lumaSouth);

    bool isHorizontal = edgeHorizontal >= edgeVertical;

    float luma1 = isHorizontal ? lumaNorth : lumaWest;
    float luma2 = isHorizontal ? lumaSouth : lumaEast;

    float gradient1 = luma1 - lumaCenter;
    float gradient2 = luma2 - lumaCenter;

    bool isGradient1Steeper = abs(gradient1) >= abs(gradient2);

    float gradient = max(abs(gradient1), abs(gradient2));
    float gradientThreshold = gradient * 0.25;

    float stepLength = isHorizontal ? texelSize.y : texelSize.x;

    float lumaLocalAverage;

    if (isGradient1Steeper)
    {
        stepLength = -stepLength;
        lumaLocalAverage = 0.5 * (luma1 + lumaCenter);
    }
    else
    {
        lumaLocalAverage = 0.5 * (luma2 + lumaCenter);
    }

    vec2 currentUV = fragUV;

    if (isHorizontal)
    {
        currentUV.y += stepLength * 0.5;
    }
    else
    {
        currentUV.x += stepLength * 0.5;
    }

    vec2 edgeStep = isHorizontal ? vec2(texelSize.x, 0.0) : vec2(0.0, texelSize.y);

    vec2 uv1 = currentUV - edgeStep;
    vec2 uv2 = currentUV + edgeStep;

    float lumaEnd1 = Luma(ProcessColor(texture(sceneColor, uv1).rgb)) - lumaLocalAverage;
    float lumaEnd2 = Luma(ProcessColor(texture(sceneColor, uv2).rgb)) - lumaLocalAverage;

    bool reachedEnd1 = abs(lumaEnd1) >= gradientThreshold;
    bool reachedEnd2 = abs(lumaEnd2) >= gradientThreshold;

    for (int i = 0; i < searchSteps && (!reachedEnd1 || !reachedEnd2); ++i)
    {
        if (!reachedEnd1)
        {
            uv1 -= edgeStep;
            lumaEnd1 = Luma(ProcessColor(texture(sceneColor, uv1).rgb)) - lumaLocalAverage;
            reachedEnd1 = abs(lumaEnd1) >= gradientThreshold;
        }

        if (!reachedEnd2)
        {
            uv2 += edgeStep;
            lumaEnd2 = Luma(ProcessColor(texture(sceneColor, uv2).rgb)) - lumaLocalAverage;
            reachedEnd2 = abs(lumaEnd2) >= gradientThreshold;
        }
    }

    float distance1 = isHorizontal ? fragUV.x - uv1.x : fragUV.y - uv1.y;
    float distance2 = isHorizontal ? uv2.x - fragUV.x : uv2.y - fragUV.y;

    bool isDirection1 = distance1 < distance2;

    float distanceFinal = min(distance1, distance2);
    float edgeThickness = distance1 + distance2;

    float edgeOffset = 0.5 - distanceFinal / edgeThickness;

    float lumaEnd = isDirection1 ? lumaEnd1 : lumaEnd2;

    bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;
    bool correctVariation = (lumaEnd < 0.0) != isLumaCenterSmaller;

    if (!correctVariation)
    {
        edgeOffset = 0.0;
    }

    float lumaAverage = (2.0 * (lumaNorth + lumaSouth + lumaWest + lumaEast) + lumaNorthWest + lumaNorthEast + lumaSouthWest + lumaSouthEast) / 12.0;

    float subpixelOffset = abs(lumaAverage - lumaCenter) / contrast;
    subpixelOffset = clamp(subpixelOffset, 0.0, 1.0);
    subpixelOffset = (-2.0 * subpixelOffset + 3.0) * subpixelOffset * subpixelOffset;
    subpixelOffset = subpixelOffset * subpixelOffset * subpixelQuality;

    float finalOffset = max(edgeOffset, subpixelOffset);

    vec2 finalUV = fragUV;

    if (isHorizontal)
    {
        finalUV.y += finalOffset * stepLength;
    }
    else
    {
        finalUV.x += finalOffset * stepLength;
    }

    vec3 finalColor = ProcessColor(texture(sceneColor, finalUV).rgb);

    outColor = vec4(finalColor, 1.0);
}
