// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <Window/Cocoa/CocoaWindow.h>
#include "AppleWindowFactory.h"
#include <memory>

#if MX_PLATFORM_APPLE
std::unique_ptr<AbstractWindow> CreateAppleWindow()
{
    return std::make_unique<CocoaWindow>();
}
#endif
