// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include "WindowConfig.h"

enum class WindowEventType : uint8
{
    RESIZED,
    MINIMIZED,
    MAXIMIZED,
    RESTORED,
    FOCUS_GAINED,
    FOCUS_LOST,
    CLOSING
};

struct WindowEvent
{
    WindowEventType eventType{};
    WindowConfig windowConfig{};
};

using WindowEventCallback = void(*)(const WindowEvent&, void* user);
