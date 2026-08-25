// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>

#if !MX_PLATFORM_APPLE
    #error CocoaWindow.h header should be included only on Apple platform
#endif

#include <MX/Window/AbstractWindow.h>
#include <memory>

class CocoaWindow final : public AbstractWindow
{
public:
    /* CocoaWindow */
    CocoaWindow();
    ~CocoaWindow() override;

    /* IWindow */
    void SetSize(const Size2i& size) override;
    void SetTitle(std::string_view title) override;
    void SetState(const WindowState& state) override;
    void SetVisibility(bool visible) override;
    void SetResizable(bool resizable) override;

    /* AbstractWindow */
    bool Create(const WindowConfig& createInfo = WindowConfig()) override;
    void Destroy() override;

    void PollEvents() override;

    /* SurfaceHost */
    NativeWindowHandle GetNativeHandle() const override;

private:
    struct CocoaWindowImpl;
    std::unique_ptr<CocoaWindowImpl> _impl;

    void DispatchEvent(WindowEventType type);
};
