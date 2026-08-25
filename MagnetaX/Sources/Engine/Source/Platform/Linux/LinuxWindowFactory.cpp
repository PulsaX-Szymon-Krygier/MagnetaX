// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "LinuxWindowFactory.h"

#if MX_PLATFORM_LINUX
#if MX_PLATFORM_X11
    #include <Window/X11/X11Window.h>
#endif

#include <cstdlib>
#include <memory>

namespace
{
    const char* GetEnv(const char* name)
    {
        const char* v = std::getenv(name);

        return (v && *v) ? v : nullptr;
    }
}

std::unique_ptr<AbstractWindow> CreateLinuxWindow()
{
    const char* waylandDisplay = GetEnv("WAYLAND_DISPLAY");
    const char* display = GetEnv("DISPLAY");

    /*
    if(waylandDisplay) return nullptr;
    else if(display) return std::make_unique<X11Window>(info);
    else return nullptr;
    */

    if (waylandDisplay) return nullptr;

    #if MX_PLATFORM_X11
    if (display) return std::make_unique<X11Window>();
    #else
    (void)display;
    #endif

    return nullptr;
}
#endif
