// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Core/Math/Size.h>
#include "WindowState.h"
#include <string>

struct WindowConfig
{
    Size2i size = Size2i(512);
    std::string title = "MagnetaX";
    WindowState state = WindowState::NORMAL;
    bool visible = false;
    bool resizable = false;
};
