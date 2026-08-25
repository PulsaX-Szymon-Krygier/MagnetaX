// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Core/Math/Vector.h>
#include <MX/Graphics/LightType.h>

struct RenderLight
{
    LightType type = LightType::POINT;

    Vector3f position{ 0.0f };
    Vector3f direction{ 0.0f, 0.0f, -1.0f };

    Vector3f color{ 1.0f };
    float32 intensity = 1.0f;

    float32 range = 10.0f;

    float32 innerConeAngle = 20.0f;
    float32 outerConeAngle = 30.0f;

    bool castsShadows = false;
};
