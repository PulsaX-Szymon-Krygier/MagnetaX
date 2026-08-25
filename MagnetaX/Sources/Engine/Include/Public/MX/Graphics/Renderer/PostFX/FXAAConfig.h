// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>

struct FXAAConfig
{
    bool enabled = true;

    float32 edgeThresholdMin = 0.03f;
    float32 edgeThreshold = 0.125f;

    uint16 searchSteps = 12;
};
