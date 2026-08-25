// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>

#if !MX_PLATFORM_LINUX
    #error LinuxWindowFactory.h header should be included only on Linux platform
#endif

#include <MX/Window/AbstractWindow.h>
#include <memory>

std::unique_ptr<AbstractWindow> CreateLinuxWindow();
