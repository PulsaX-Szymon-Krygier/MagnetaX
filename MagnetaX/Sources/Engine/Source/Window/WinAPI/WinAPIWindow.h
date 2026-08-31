// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>

#if !MX_PLATFORM_WINDOWS
    #error WinAPIWindow.h header should be included only on Windows platform
#endif

#include <MX/Window/AbstractWindow.h>

#ifndef NOMINMAX
    #define NOMINMAX
#endif

#include <Windows.h>
#include <string_view>

class WinAPIWindow final : public AbstractWindow
{
public:
    WinAPIWindow();
    ~WinAPIWindow() override;

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
    HINSTANCE hInstance = nullptr;
    HWND hWnd = nullptr;

    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
