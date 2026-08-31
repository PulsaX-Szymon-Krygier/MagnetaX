// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#if !MX_PLATFORM_X11
    #error This header should be included only on X11 platform
#endif

#include <MX/Core/CoreMinimal.h>
#include <MX/Window/AbstractWindow.h>

class X11Window final : public AbstractWindow
{
public:
    X11Window();
    ~X11Window() override;

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
    void* xDisplay = nullptr;
    uint64 xWindow = 0;

    uint64 wmProtocolsAtom = 0;
    uint64 wmDeleteAtom = 0;
    uint64 netWMStateAtom = 0;
    uint64 netWMStateMaximizedHorzAtom = 0;
    uint64 netWMStateMaximizedVertAtom = 0;
    uint64 netWMStateHiddenAtom = 0;

    void SendNetWMState(int32 action, uint64 firstState, uint64 secondState = 0);
    void UpdateWindowState();
};
