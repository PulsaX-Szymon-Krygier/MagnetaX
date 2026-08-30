// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Input/InputFeed.h>
#include <Input/WinAPI/WinAPIInput.h>
#include "WinAPIWindow.h"
#include <string>

#if MX_PLATFORM_WINDOWS
namespace
{
    constexpr const wchar_t* WINDOW_CLASS_NAME = L"MagnetaX_WinAPIWindowClass";

    std::wstring UTF8ToWide(std::string_view string)
    {
        if (string.empty()) return L"";

        const int32 length = MultiByteToWideChar(CP_UTF8, 0, string.data(), static_cast<int32>(string.size()), nullptr, 0);
        if (length <= 0) return L"";

        std::wstring result(static_cast<usize>(length), L'\0');

        if (MultiByteToWideChar(CP_UTF8, 0, string.data(), static_cast<int32>(string.size()), result.data(), length) != length) return L"";

        return result;
    }
}

WinAPIWindow::WinAPIWindow() = default;
WinAPIWindow::~WinAPIWindow() = default;

void WinAPIWindow::SetSize(const Size2i& size)
{
    if (!hWnd) return;
    if (size.width == 0 || size.height == 0) return;

    RECT sizeRect{ 0, 0, (LONG)size.width, (LONG)size.height };

    DWORD currentStyle = (DWORD)GetWindowLongPtrW(hWnd, GWL_STYLE);
    AdjustWindowRect(&sizeRect, currentStyle, FALSE);

    const Size2i buffSize(sizeRect.right - sizeRect.left, sizeRect.bottom - sizeRect.top);
    SetWindowPos(hWnd, nullptr, 0, 0, buffSize.width, buffSize.height, SWP_NOMOVE | SWP_NOZORDER);
}

void WinAPIWindow::SetTitle(std::string_view title)
{
    if (!hWnd) return;

    std::wstring titleW = UTF8ToWide(title);

    if (SetWindowTextW(hWnd, titleW.c_str())) config.title = std::string(title);
}

void WinAPIWindow::SetState(const WindowState& state)
{
    if (!hWnd) return;

    int32 cmdShow = SW_RESTORE;

    if (state == WindowState::MINIMIZED) cmdShow = SW_MINIMIZE;
    else if (state == WindowState::MAXIMIZED) cmdShow = SW_MAXIMIZE;

    ShowWindow(hWnd, cmdShow);
}

void WinAPIWindow::SetVisibility(bool visible)
{
    if (!hWnd) return;

    if (visible)
    {
        const BOOL minimized = IsIconic(hWnd);
        const BOOL maximized = IsZoomed(hWnd);

        if (maximized) ShowWindow(hWnd, SW_SHOWMAXIMIZED);
        else if (minimized) ShowWindow(hWnd, SW_RESTORE);
        else ShowWindow(hWnd, SW_SHOW);
    }
    else ShowWindow(hWnd, SW_HIDE);

    UpdateWindow(hWnd);

    config.visible = (bool)IsWindowVisible(hWnd);
}

void WinAPIWindow::SetResizable(bool resizable)
{
    if (!hWnd) return;

    const Size2i currentSize = config.size;
    DWORD style = (DWORD)GetWindowLongPtrW(hWnd, GWL_STYLE);

    if (resizable) style |= (WS_THICKFRAME | WS_MAXIMIZEBOX);
    else style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);

    SetWindowLongPtrW(hWnd, GWL_STYLE, (LONG_PTR)style);
    SetWindowPos(hWnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    config.resizable = resizable;

    SetSize(currentSize);
}

bool WinAPIWindow::Create(const WindowConfig& createInfo)
{
    if (hWnd) return false;
    if (createInfo.size.width == 0 || createInfo.size.height == 0) return false;

    hInstance = GetModuleHandleW(nullptr);

    // Create and register WinAPI Window class
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = &WinAPIWindow::StaticWndProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    windowClass.lpszClassName = WINDOW_CLASS_NAME;

    RegisterClassExW(&windowClass);

    // Set initial configuration
    RECT initialRect{ 0, 0, (LONG)createInfo.size.width, (LONG)createInfo.size.height };
    std::wstring initialTitle = UTF8ToWide(createInfo.title);

    DWORD initialStyle = WS_OVERLAPPEDWINDOW;

    if (!createInfo.resizable) initialStyle &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);

    AdjustWindowRect(&initialRect, initialStyle, FALSE);

    // Create window
    hWnd = CreateWindowExW(
        0, WINDOW_CLASS_NAME,
        initialTitle.c_str(), initialStyle,
        CW_USEDEFAULT, CW_USEDEFAULT,
        (initialRect.right - initialRect.left), (initialRect.bottom - initialRect.top),
        nullptr, nullptr, hInstance, this
    );

    if (!hWnd)
    {
        hInstance = nullptr;

        return false;
    }

    // Complete and save configuration
    config = createInfo;
    SetVisibility(createInfo.visible);

    return true;
}

void WinAPIWindow::Destroy()
{
    if (hWnd)
    {
        DestroyWindow(hWnd);
        hWnd = nullptr;
    }
}

void WinAPIWindow::PollEvents()
{
    MSG msg{};

    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

NativeWindowHandle WinAPIWindow::GetNativeHandle() const
{
    NativeWindowHandle nativeHandle{};
    nativeHandle.system = NativeWindowSystem::WIN_API;
    nativeHandle.window = (void*)hWnd;
    nativeHandle.display = (void*)hInstance;

    return nativeHandle;
}

void WinAPIWindow::DispatchEvent(WindowEventType type)
{
    WindowEvent event{};
    event.eventType = type;
    event.windowConfig = config;

    for (const auto& subscriber : subscribers)
    {
        if (subscriber.callback) subscriber.callback(event, subscriber.user);
    }
}

LRESULT CALLBACK WinAPIWindow::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    WinAPIWindow* window = nullptr;

    if (msg == WM_NCCREATE)
    {
        CREATESTRUCTW* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        window = reinterpret_cast<WinAPIWindow*>(createStruct->lpCreateParams);

        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)window);
    }
    else
    {
        window = reinterpret_cast<WinAPIWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    return window ? window->WndProc(hwnd, msg, wParam, lParam) : DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT WinAPIWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (inputFeed)
    {
        WinAPIInput::ProcessMessage(*inputFeed, msg, wParam, lParam);
    }

    switch (msg)
    {
    case WM_CLOSE:
    {
        DispatchEvent(WindowEventType::CLOSING);

        return 0;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);

        return 0;
    }
    case WM_SIZE:
    {
        const Size2i previousSize = config.size;
        const WindowState previousState = config.state;

        RECT clientRect{};
        GetClientRect(hwnd, &clientRect);

        const Size2i newSize((clientRect.right - clientRect.left), (clientRect.bottom - clientRect.top));
        WindowState newState = WindowState::NORMAL;

        if (wParam == SIZE_MINIMIZED) newState = WindowState::MINIMIZED;
        else if (wParam == SIZE_MAXIMIZED) newState = WindowState::MAXIMIZED;

        config.size = newSize;
        config.state = newState;

        if (newState != WindowState::MINIMIZED && newSize != previousSize) DispatchEvent(WindowEventType::RESIZED);

        if (newState != previousState)
        {
            if (newState == WindowState::MINIMIZED) DispatchEvent(WindowEventType::MINIMIZED);
            else if (newState == WindowState::MAXIMIZED) DispatchEvent(WindowEventType::MAXIMIZED);
            else DispatchEvent(WindowEventType::RESTORED);
        }

        return 0;
    }
    case WM_SETFOCUS:
    {
        DispatchEvent(WindowEventType::FOCUS_GAINED);

        return 0;
    }
    case WM_KILLFOCUS:
    {
        DispatchEvent(WindowEventType::FOCUS_LOST);

        return 0;
    }
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
#endif
