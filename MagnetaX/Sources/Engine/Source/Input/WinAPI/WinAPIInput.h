// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>

#if !MX_PLATFORM_WINDOWS
  #error WinAPIInput.h header should be included only on Windows platform
#endif

#include <MX/Input/InputFeed.h>

#ifndef NOMINMAX
  #define NOMINMAX
#endif
#include <Windows.h>

struct WinAPIInput
{
    static bool ProcessMessage(InputFeed& inputFeed, UINT msg, WPARAM wParam, LPARAM lParam);
};
