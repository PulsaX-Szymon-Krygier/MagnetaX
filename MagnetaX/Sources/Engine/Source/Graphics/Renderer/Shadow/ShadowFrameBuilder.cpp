// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "ShadowFrameBuilder.h"
#include <MX/Core/Math/MathUtil.h>
#include <Graphics/Renderer/Scene/RenderSceneData.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace
{
    struct LightSpaceBounds
    {
        float32 minX, maxX;
        float32 minY, maxY;
        float32 minZ, maxZ;
    };

    std::array<float32, MX_GRAPHICS_DIRECTIONAL_SHADOW_CASCADE_COUNT> CalculateCascadeSplits(float32 nearPlane, float32 farPlane, float32 lambda)
    {
        std::array<float32, MX_GRAPHICS_DIRECTIONAL_SHADOW_CASCADE_COUNT> splits{};

        for (uint32 i = 0; i < MX_GRAPHICS_DIRECTIONAL_SHADOW_CASCADE_COUNT; ++i)
        {
            const float32 p = (float32)(i + 1) / (float32)MX_GRAPHICS_DIRECTIONAL_SHADOW_CASCADE_COUNT;

            const float32 logarithmicSplit = nearPlane * std::pow(farPlane / nearPlane, p);
            const float32 linearSplit = MathUtil::Lerp(nearPlane, farPlane, p);

            splits[i] = MathUtil::Lerp(linearSplit, logarithmicSplit, lambda);
        }

        return splits;
    }

    std::array<Vector3f, 8> CalculateFrustumCornersWorldSpace(const Matrix4f& viewProjInv)
    {
        std::array<Vector3f, 8> corners{};
        uint32 index = 0;

        for (uint32 z = 0; z < 2; ++z)
        {
            for (uint32 y = 0; y < 2; ++y)
            {
                for (uint32 x = 0; x < 2; ++x)
                {
                    const Vector4f ndcPos(x == 0 ? -1.0f : 1.0f, y == 0 ? -1.0f : 1.0f, (float32)z, 1.0f);

                    Vector4f worldPos = viewProjInv * ndcPos;
                    worldPos /= worldPos.w;

                    corners[index++] = Vector3f(worldPos.x, worldPos.y, worldPos.z);
                }
            }
        }

        return corners;
    }

    std::array<Vector3f, 8> CalculateCascadeFrustumCorners(const std::array<Vector3f, 8>& frustumCorners, float32 cameraNear,
        float32 cameraFar, float32 cascadeNear, float32 cascadeFar)
    {
        std::array<Vector3f, 8> cascadeCorners{};

        const float32 nearT = (cascadeNear - cameraNear) / (cameraFar - cameraNear);
        const float32 farT = (cascadeFar - cameraNear) / (cameraFar - cameraNear);

        for (uint32 i = 0; i < 4; ++i)
        {
            cascadeCorners[i] = MathUtil::Lerp(frustumCorners[i], frustumCorners[i + 4], nearT);
            cascadeCorners[i + 4] = MathUtil::Lerp(frustumCorners[i], frustumCorners[i + 4], farT);
        }

        return cascadeCorners;
    }

    Vector3f CalculateFrustumCenter(const std::array<Vector3f, 8>& corners)
    {
        Vector3f center(0.0f);

        for (const Vector3f& corner : corners) center += corner;

        return center / (float32)corners.size();
    }

    LightSpaceBounds CalculateLightSpaceBounds(const std::array<Vector3f, 8>& corners, const Matrix4f& lightView)
    {
        LightSpaceBounds bounds{};
        bounds.minX = bounds.minY = bounds.minZ = std::numeric_limits<float32>::max();
        bounds.maxX = bounds.maxY = bounds.maxZ = std::numeric_limits<float32>::lowest();

        for (const Vector3f& corner : corners)
        {
            const Vector4f lightSpacePosition = lightView * Vector4f(corner, 1.0f);

            bounds.minX = std::min(bounds.minX, lightSpacePosition.x);
            bounds.maxX = std::max(bounds.maxX, lightSpacePosition.x);

            bounds.minY = std::min(bounds.minY, lightSpacePosition.y);
            bounds.maxY = std::max(bounds.maxY, lightSpacePosition.y);

            bounds.minZ = std::min(bounds.minZ, lightSpacePosition.z);
            bounds.maxZ = std::max(bounds.maxZ, lightSpacePosition.z);
        }

        return bounds;
    }

    int32 FindDirectionalShadowLight(const RenderSceneData& sceneData)
    {
        for (uint32 i = 0; i < sceneData.lights.size(); ++i)
        {
            const RenderLight& light = sceneData.lights[i];

            if (light.type != LightType::DIRECTIONAL || !light.castsShadows) continue;
            if (light.direction.LengthSquared() <= MX_MATH_EPSILON_SQUARED) continue;

            return (int32)i;
        }

        return MX_GRAPHICS_INVALID_SHADOW_LIGHT_INDEX;
    }

    int32 FindSpotShadowLight(const RenderSceneData& sceneData)
    {
        for (uint32 i = 0; i < sceneData.lights.size(); ++i)
        {
            const RenderLight& light = sceneData.lights[i];

            if (light.type != LightType::SPOT || !light.castsShadows) continue;
            if (light.direction.LengthSquared() <= MX_MATH_EPSILON_SQUARED) continue;
            if (light.range <= 0.0f || light.outerConeAngle <= 0.0f) continue;

            return (int32)i;
        }

        return MX_GRAPHICS_INVALID_SHADOW_LIGHT_INDEX;
    }
}

ShadowFrameData BuildShadowFrameData(const RenderSceneData& sceneData, const ShadowConfig& config)
{
    ShadowFrameData shadowData{};

    if (sceneData.cameraNearPlane <= 0.0f || sceneData.cameraFarPlane <= sceneData.cameraNearPlane) return shadowData;

    const int32 directionalLightIndex = FindDirectionalShadowLight(sceneData);

    if (directionalLightIndex != MX_GRAPHICS_INVALID_SHADOW_LIGHT_INDEX && config.directional.resolution > 0)
    {
        const float32 shadowFarPlane = std::min(sceneData.cameraFarPlane, config.directional.distance);

        if (shadowFarPlane > sceneData.cameraNearPlane)
        {
            shadowData.directional.lightIndex = directionalLightIndex;

            const RenderLight& light = sceneData.lights[directionalLightIndex];
            const float32 splitLambda = std::clamp(config.directional.splitLambda, 0.0f, 1.0f);
            const float32 blendRatio = std::max(config.directional.cascadeBlendRatio, 0.0f);

            shadowData.directional.splits = CalculateCascadeSplits(sceneData.cameraNearPlane, shadowFarPlane, splitLambda);

            const std::array<Vector3f, 8> frustumCorners = CalculateFrustumCornersWorldSpace(sceneData.viewProjectionInversed);

            Vector3f lightDirection = light.direction;
            lightDirection.Normalize();

            Vector3f shadowUp(0.0f, 1.0f, 0.0f);
            if (std::abs(Vector3f::Dot(lightDirection, shadowUp)) > 0.99f) shadowUp = Vector3f(1.0f, 0.0f, 0.0f);

            for (uint32 i = 0; i < MX_GRAPHICS_DIRECTIONAL_SHADOW_CASCADE_COUNT; ++i)
            {
                const float32 cascadeNear = i == 0 ? sceneData.cameraNearPlane : shadowData.directional.splits[i - 1];
                const float32 cascadeFar = shadowData.directional.splits[i];

                const float32 cascadeLength = cascadeFar - cascadeNear;
                const float32 blendWidth = i + 1 < MX_GRAPHICS_DIRECTIONAL_SHADOW_CASCADE_COUNT ? cascadeLength * blendRatio : 0.0f;
                const float32 cascadeRenderFar = std::min(cascadeFar + blendWidth, shadowFarPlane);

                shadowData.directional.blendWidths[i] = cascadeRenderFar - cascadeFar;

                const std::array<Vector3f, 8> cascadeCorners = CalculateCascadeFrustumCorners(
                    frustumCorners, sceneData.cameraNearPlane, sceneData.cameraFarPlane, cascadeNear, cascadeRenderFar
                );

                const Vector3f shadowTarget = CalculateFrustumCenter(cascadeCorners);

                float32 cascadeRadius = 0.0f;

                for (const Vector3f& corner : cascadeCorners)
                {
                    cascadeRadius = std::max(cascadeRadius, Vector3f::Distance(shadowTarget, corner));
                }

                cascadeRadius = std::ceil(cascadeRadius * 16.0f) / 16.0f;

                const float32 shadowDiameter = cascadeRadius * 2.0f;
                const float32 worldUnitsPerTexel = shadowDiameter / (float32)config.directional.resolution;

                const Vector3f shadowEye = shadowTarget - lightDirection * (cascadeRadius * 2.0f);

                Matrix4f lightView = Matrix4f::ViewLookAtRightHanded(shadowEye, shadowTarget, shadowUp);

                lightView.m03 = std::round(lightView.m03 / worldUnitsPerTexel) * worldUnitsPerTexel;
                lightView.m13 = std::round(lightView.m13 / worldUnitsPerTexel) * worldUnitsPerTexel;

                const LightSpaceBounds bounds = CalculateLightSpaceBounds(cascadeCorners, lightView);

                const float32 depthPadding = cascadeRadius;
                const float32 shadowNear = std::max(0.01f, -bounds.maxZ - depthPadding);
                const float32 shadowFar = -bounds.minZ + depthPadding;

                const float32 shadowDepthRange = shadowFar - shadowNear;

                if (shadowDepthRange <= MX_MATH_EPSILON)
                {
                    shadowData.directional.lightIndex = MX_GRAPHICS_INVALID_SHADOW_LIGHT_INDEX;
                    break;
                }

                shadowData.directional.biases[i] = worldUnitsPerTexel / shadowDepthRange;

                const Matrix4f lightProjection = Matrix4f::ProjectionOrthographicRightHanded(
                    shadowDiameter, shadowDiameter, shadowNear, shadowFar
                );

                shadowData.directional.viewProjs[i] = lightProjection * lightView;
            }
        }
    }

    const int32 spotLightIndex = FindSpotShadowLight(sceneData);

    if (spotLightIndex != MX_GRAPHICS_INVALID_SHADOW_LIGHT_INDEX && config.spot.resolution > 0)
    {
        const RenderLight& light = sceneData.lights[spotLightIndex];
        const float32 shadowNear = std::max(config.spot.nearPlane, 0.001f);

        if (shadowNear < light.range)
        {
            shadowData.spot.lightIndex = spotLightIndex;

            Vector3f lightDirection = light.direction;
            lightDirection.Normalize();

            Vector3f shadowUp(0.0f, 1.0f, 0.0f);
            if (std::abs(Vector3f::Dot(lightDirection, shadowUp)) > 0.99f) shadowUp = Vector3f(1.0f, 0.0f, 0.0f);

            const Vector3f shadowEye = light.position;
            const Vector3f shadowTarget = shadowEye + lightDirection;

            const Matrix4f lightView = Matrix4f::ViewLookAtRightHanded(shadowEye, shadowTarget, shadowUp);
            const float32 fov = MathUtil::DegToRad(light.outerConeAngle * 2.0f);

            const Matrix4f lightProjection = Matrix4f::ProjectionPerspectiveRightHanded(fov, 1.0f, shadowNear, light.range);

            shadowData.spot.viewProj = lightProjection * lightView;
        }
    }

    return shadowData;
}
