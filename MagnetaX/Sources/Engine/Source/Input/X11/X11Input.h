// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>

#if !MX_PLATFORM_X11
#error X11Input.h header should be included only on X11 platform
#endif

#include <MX/Input/InputFeed.h>
#include <X11/Xlib.h>

struct X11Input
{
    static void Initialize(Display* display);
    static bool ProcessEvent(InputFeed& inputFeed, XEvent& event);
};
