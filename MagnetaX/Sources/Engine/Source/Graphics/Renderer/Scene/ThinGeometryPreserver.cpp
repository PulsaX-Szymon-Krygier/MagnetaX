// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "ThinGeometryPreserver.h"
#include <MX/Core/Math/MathConst.h>
#include <algorithm>
#include <cmath>

namespace
{
    bool ProjectToPixels(const Matrix4f& modelViewProj, const Vector3f& localPosition, const Size2i& renderSize, Vector2f& pixelPosition)
    {
        const Vector4f clipPosition = modelViewProj * Vector4f(localPosition, 1.0f);
        if (clipPosition.w <= MX_MATH_EPSILON) return false;

        const float32 inverseW = 1.0f / clipPosition.w;
        const float32 ndcX = clipPosition.x * inverseW;
        const float32 ndcY = clipPosition.y * inverseW;

        pixelPosition.x = (ndcX * 0.5f + 0.5f) * (float32)renderSize.width;
        pixelPosition.y = (ndcY * 0.5f + 0.5f) * (float32)renderSize.height;

        return true;
    }
}

Matrix4f ThinGeometryPreserver::ComputeRasterModel(const Matrix4f& model, const Vector3f& localBoundsMin,
    const Vector3f& localBoundsMax, const Matrix4f& viewProj, const Size2i& renderSize)
{
    if (renderSize.width == 0 || renderSize.height == 0) return model;

    const Vector3f localCenter = (localBoundsMin + localBoundsMax) * 0.5f;
    const Vector3f localHalfSize = (localBoundsMax - localBoundsMin) * 0.5f;

    const Vector3f axisOffsets[3] =
    {
        Vector3f(localHalfSize.x, 0.0f, 0.0f),
        Vector3f(0.0f, localHalfSize.y, 0.0f),
        Vector3f(0.0f, 0.0f, localHalfSize.z)
    };

    uint32 thinAxis = 0;
    float32 thinWorldSize = 0.0f;
    bool thinAxisFound = false;

    for (uint32 axis = 0; axis < 3; ++axis)
    {
        const Vector3f& localOffset = axisOffsets[axis];

        if (localOffset.LengthSquared() <= MX_MATH_EPSILON_SQUARED) continue;

        const Vector4f worldOffset = model * Vector4f(localOffset, 0.0f);
        const float32 worldSize = 2.0f * Vector3f(worldOffset.x, worldOffset.y, worldOffset.z).Length();

        if (worldSize <= MX_MATH_EPSILON) continue;

        if (thinAxisFound && worldSize >= thinWorldSize) continue;

        thinAxis = axis;
        thinWorldSize = worldSize;
        thinAxisFound = true;
    }

    if (!thinAxisFound) return model;

    const Vector3f& thinOffset = axisOffsets[thinAxis];
    const Vector3f localA = localCenter - thinOffset;
    const Vector3f localB = localCenter + thinOffset;

    const Matrix4f modelViewProj = viewProj * model;

    Vector2f pixelA;
    Vector2f pixelB;

    if (!ProjectToPixels(modelViewProj, localA, renderSize, pixelA)) return model;
    if (!ProjectToPixels(modelViewProj, localB, renderSize, pixelB)) return model;

    const Vector4f centerClip = modelViewProj * Vector4f(localCenter, 1.0f);
    if (centerClip.w <= MX_MATH_EPSILON || centerClip.z < 0.0f || centerClip.z > centerClip.w) return model;

    const Vector3f projectionX(viewProj.m00, viewProj.m01, viewProj.m02);
    const Vector3f projectionY(viewProj.m10, viewProj.m11, viewProj.m12);

    const float32 pixelScaleX = projectionX.Length() * (float32)renderSize.width * 0.5f;
    const float32 pixelScaleY = projectionY.Length() * (float32)renderSize.height * 0.5f;

    const float32 referencePixels = thinWorldSize * std::max(pixelScaleX, pixelScaleY) / centerClip.w;
    const float32 projectedPixels = Vector2f::Distance(pixelA, pixelB);

    if (!std::isfinite(referencePixels) || !std::isfinite(projectedPixels)) return model;
    if (referencePixels <= MX_MATH_EPSILON) return model;

    constexpr float32 minRasterFootprintPixels = 1.0f;

    const float32 effectivePixels = std::max(projectedPixels, referencePixels);
    if (effectivePixels >= minRasterFootprintPixels) return model;

    const float32 scaleFactor = minRasterFootprintPixels / effectivePixels;

    Vector3f rasterScale(1.0f);

    if (thinAxis == 0) rasterScale.x = scaleFactor;
    else if (thinAxis == 1) rasterScale.y = scaleFactor;
    else rasterScale.z = scaleFactor;

    return model * Matrix4f::FromTranslation(localCenter) * Matrix4f::FromScale(rasterScale) * Matrix4f::FromTranslation(-localCenter);
}
