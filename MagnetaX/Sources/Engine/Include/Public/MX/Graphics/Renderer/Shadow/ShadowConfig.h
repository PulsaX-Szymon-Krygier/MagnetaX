// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>

struct DirectionalShadowConfig
{
    uint32 resolution = 2048;

    float32 distance = 100.0f;
    float32 splitLambda = 0.8f;
    float32 cascadeBlendRatio = 0.3f;
};

struct SpotShadowConfig
{
    uint32 resolution = 2048;

    float32 nearPlane = 0.1f;
};

struct ShadowConfig
{
    DirectionalShadowConfig directional{};
    SpotShadowConfig spot{};
};
