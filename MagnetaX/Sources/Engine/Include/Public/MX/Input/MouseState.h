// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include "MouseButtons.h"
#include <array>

struct MouseState
{
    std::array<bool, (usize)MouseButtons::COUNT> buttonDown{};
    std::array<bool, (usize)MouseButtons::COUNT> buttonPressed{};
    std::array<bool, (usize)MouseButtons::COUNT> buttonReleased{};

    int32 mouseX = 0;
    int32 mouseY = 0;
    int32 mouseDX = 0;
    int32 mouseDY = 0;
    int32 wheelDelta = 0;
};
