// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Core/Math/Matrix.h>
#include <array>

constexpr uint32 MX_GRAPHICS_DIRECTIONAL_SHADOW_CASCADE_COUNT = 4;
constexpr int32 MX_GRAPHICS_INVALID_SHADOW_LIGHT_INDEX = -1;

struct DirectionalShadowFrameData
{
    std::array<Matrix4f, MX_GRAPHICS_DIRECTIONAL_SHADOW_CASCADE_COUNT> viewProjs{};
    std::array<float32, MX_GRAPHICS_DIRECTIONAL_SHADOW_CASCADE_COUNT> splits{};
    std::array<float32, MX_GRAPHICS_DIRECTIONAL_SHADOW_CASCADE_COUNT> biases{};
    std::array<float32, MX_GRAPHICS_DIRECTIONAL_SHADOW_CASCADE_COUNT> blendWidths{};

    int32 lightIndex = MX_GRAPHICS_INVALID_SHADOW_LIGHT_INDEX;
};

struct SpotShadowFrameData
{
    Matrix4f viewProj{};

    int32 lightIndex = MX_GRAPHICS_INVALID_SHADOW_LIGHT_INDEX;
};

struct ShadowFrameData
{
    DirectionalShadowFrameData directional;
    SpotShadowFrameData spot;
};
