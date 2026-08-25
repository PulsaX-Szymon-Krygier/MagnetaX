// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>

enum class NativeWindowSystem : uint8
{
    UNKNOWN = 0,
    WIN_API,
    X11,
    WAYLAND, // Not even implemented yet but keep it for future use :D
    COCOA
};

struct NativeWindowHandle
{
    NativeWindowSystem system = NativeWindowSystem::UNKNOWN;
    void* display = nullptr;

    #if MX_PLATFORM_LINUX
    uint64 window = 0;
    #else
    void* window = nullptr;
    #endif
};
