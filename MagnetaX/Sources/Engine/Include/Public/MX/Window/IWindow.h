// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Core/Math/Size.h>
#include "WindowEvent.h"
#include "WindowState.h"
#include <string_view>

class IWindow
{
public:
    virtual ~IWindow() = default;

    virtual void SetSize(const Size2i& size) = 0;
    virtual Size2i GetSize() const = 0;

    virtual void SetTitle(std::string_view title) = 0;
    virtual std::string_view GetTitle() const = 0;

    virtual void SetState(const WindowState& state) = 0;
    virtual WindowState GetState() const = 0;

    virtual void SetVisibility(bool visible) = 0;
    virtual bool GetVisibility() const = 0;

    virtual void SetResizable(bool resizable) = 0;
    virtual bool GetResizable() const = 0;

    virtual uint32 Subscribe(WindowEventCallback callback, void* user) = 0;
    virtual void Unsubscribe(uint32 token) = 0;
};
