// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Core/Math/Size.h>
#include <MX/Platform/NativeWindowHandle.h>

class SurfaceHost
{
public:
    virtual ~SurfaceHost() = default;

    virtual NativeWindowHandle GetNativeHandle() const = 0;
    virtual Size2i GetSurfaceSize() const = 0;
};
