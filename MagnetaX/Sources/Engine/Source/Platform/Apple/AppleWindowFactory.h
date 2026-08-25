// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>

#if !MX_PLATFORM_APPLE
    #error AppleWindowFactory.h header should be included only on Apple platform
#endif

#include <MX/Window/AbstractWindow.h>
#include <memory>

std::unique_ptr<AbstractWindow> CreateAppleWindow();
