// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>

#if !MX_PLATFORM_APPLE
  #error CocoaInput.h header should be included only on Apple platform
#endif

#include <MX/Input/InputFeed.h>

@class NSEvent;
@class NSView;

struct CocoaInput
{
    static bool ProcessEvent(InputFeed& inputFeed, NSEvent* event, NSView* view);
};
