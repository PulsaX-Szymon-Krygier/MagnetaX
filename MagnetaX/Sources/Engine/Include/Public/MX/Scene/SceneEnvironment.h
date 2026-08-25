// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Core/Math/Vector.h>

struct SceneEnvironment
{
    Vector3f backgroundColor{ 0.0f };
    Vector3f ambientLightColor{ 1.0f };
    float32 ambientLightIntensity = 0.1f;
};
