// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include "KeyboardKeys.h"
#include <array>

struct KeyboardState
{
    std::array<bool, (usize)KeyboardKeys::COUNT> keyDown{};
    std::array<bool, (usize)KeyboardKeys::COUNT> keyPressed{};
    std::array<bool, (usize)KeyboardKeys::COUNT> keyReleased{};
};
