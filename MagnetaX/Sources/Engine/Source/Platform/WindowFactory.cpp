// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Platform/WindowFactory.h>
#include <MX/Core/Platform.h>

#if MX_PLATFORM_WINDOWS
    #include "Windows/WindowsWindowFactory.h"
#endif

#if MX_PLATFORM_LINUX
    #include "Linux/LinuxWindowFactory.h"
#endif

#if MX_PLATFORM_APPLE
    #include "Apple/AppleWindowFactory.h"
#endif

std::unique_ptr<AbstractWindow> CreatePlatformWindow()
{
    #if MX_PLATFORM_WINDOWS
    return CreateWindowsWindow();
    #elif MX_PLATFORM_LINUX
    return CreateLinuxWindow();
    #elif MX_PLATFORM_APPLE
    return CreateAppleWindow();
    #else
    return nullptr; //... and what? Handle it in app later, display message or something
    #endif
}
