// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <Window/WinAPI/WinAPIWindow.h>
#include "WindowsWindowFactory.h"
#include <memory>

#if MX_PLATFORM_WINDOWS
#undef CreateWindow

std::unique_ptr<AbstractWindow> CreateWindowsWindow()
{
    return std::make_unique<WinAPIWindow>();
}
#endif
