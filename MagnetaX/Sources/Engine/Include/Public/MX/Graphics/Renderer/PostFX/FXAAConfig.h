// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>

struct FXAAConfig
{
    bool enabled = false;

    float32 contrastThreshold = 0.025f;
    float32 relativeThreshold = 0.165f;
    float32 subpixelBlending = 0.75f;
};
