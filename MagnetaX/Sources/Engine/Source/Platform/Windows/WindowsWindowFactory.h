// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>

#if !MX_PLATFORM_WINDOWS
    #error WindowsWindowFactory.h header should be included only on Windows platform
#endif

#include <MX/Window/AbstractWindow.h>
#include <memory>

std::unique_ptr<AbstractWindow> CreateWindowsWindow();
