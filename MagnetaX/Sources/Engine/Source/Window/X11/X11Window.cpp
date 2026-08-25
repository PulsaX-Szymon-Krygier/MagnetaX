// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "X11Window.h"

#if MX_PLATFORM_X11
#include <MX/Core/Types.h>
#include <Input/X11/X11Input.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <string>

namespace
{
    Display* AsDisplay(void* p)
    {
        return reinterpret_cast<Display*>(p);
    }
}

X11Window::X11Window() = default;
X11Window::~X11Window() = default;

bool X11Window::Create(const WindowConfig& createInfo)
{
    if (xDisplay) return false;
    if (createInfo.size.width == 0 || createInfo.size.height == 0) return false;

    // Open current display
    Display* display = XOpenDisplay(nullptr);
    if (!display) return false;

    xDisplay = display;

    // Create window
    const int32 screen = (int32)DefaultScreen(display);
    Size2i size = createInfo.size;

    Window window = XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0, (unsigned)size.width, (unsigned)size.height,
        0, BlackPixel(display, screen), BlackPixel(display, screen));

    if (!window)
    {
        XCloseDisplay(display);
        xDisplay = nullptr;

        return false;
    }

    xWindow = (uint64)window;

    // Configure input and atoms
    XSelectInput(display, window, ExposureMask | StructureNotifyMask | PropertyChangeMask | FocusChangeMask |
        KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

    X11Input::Initialize(display);

    Atom wmProtocols = XInternAtom(display, "WM_PROTOCOLS", False);
    Atom wmDelete = XInternAtom(display, "WM_DELETE_WINDOW", False);
    netWMStateAtom = (uint64)XInternAtom(display, "_NET_WM_STATE", False);
    netWMStateMaximizedHorzAtom = (uint64)XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    netWMStateMaximizedVertAtom = (uint64)XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    netWMStateHiddenAtom = (uint64)XInternAtom(display, "_NET_WM_STATE_HIDDEN", False);

    XSetWMProtocols(display, window, &wmDelete, 1);

    wmProtocolsAtom = (uint64)wmProtocols;
    wmDeleteAtom = (uint64)wmDelete;

    // Complete and save config
    config = createInfo;
    config.size = size;
    config.visible = false;
    config.state = WindowState::NORMAL;

    SetResizable(createInfo.resizable);
    SetTitle(createInfo.title);

    if (createInfo.visible)
    {
        if (createInfo.state == WindowState::MAXIMIZED)
        {
            Atom states[] =
            {
                (Atom)netWMStateMaximizedHorzAtom,
                (Atom)netWMStateMaximizedVertAtom
            };

            XChangeProperty(display, window, (Atom)netWMStateAtom, XA_ATOM, 32, PropModeReplace,
                reinterpret_cast<const unsigned char*>(states), 2);
        }

        SetVisibility(true);

        if (createInfo.state == WindowState::MINIMIZED) SetState(WindowState::MINIMIZED);
    }

    return true;
}

void X11Window::Destroy()
{
    Display* display = AsDisplay(xDisplay);
    if (!display) return;

    if (xWindow) XDestroyWindow(display, (Window)xWindow);
    XCloseDisplay(display);

    xDisplay = nullptr;
    xWindow = 0;
}

void X11Window::SetSize(const Size2i& size)
{
    Display* display = AsDisplay(xDisplay);

    if (!display || !xWindow) return;
    if (size.width == 0 || size.height == 0) return;

    if (!config.resizable)
    {
        XSizeHints hints{};
        hints.flags = PMinSize | PMaxSize;
        hints.min_width = hints.max_width = size.width;
        hints.min_height = hints.max_height = size.height;

        XSetWMNormalHints(display, (Window)xWindow, &hints);
    }

    XResizeWindow(display, (Window)xWindow, size.width, size.height);
    XFlush(display);
}

void X11Window::SetTitle(std::string_view title)
{
    Display* display = AsDisplay(xDisplay);
    if (!display || !xWindow) return;

    const std::string buffTitle(title);

    XStoreName(display, (Window)xWindow, buffTitle.c_str());
    XFlush(display);

    config.title = buffTitle;
}

void X11Window::SetState(const WindowState& state)
{
    Display* display = AsDisplay(xDisplay);
    if (!display || !xWindow) return;

    switch (state)
    {
    case WindowState::MINIMIZED:
    {
        if (config.visible) XIconifyWindow(display, (Window)xWindow, DefaultScreen(display));

        break;
    }
    case WindowState::MAXIMIZED:
    {
        if (config.visible) XMapWindow(display, (Window)xWindow);
        SendNetWMState(1, netWMStateMaximizedHorzAtom, netWMStateMaximizedVertAtom);

        break;
    }
    case WindowState::NORMAL:
    {
        SendNetWMState(0, netWMStateMaximizedHorzAtom, netWMStateMaximizedVertAtom);
        if (config.visible) XMapWindow(display, (Window)xWindow);

        break;
    }
    }

    XFlush(display);
}

void X11Window::SetVisibility(bool visible)
{
    Display* display = AsDisplay(xDisplay);
    if (!display || !xWindow) return;

    if (visible) XMapWindow(display, (Window)xWindow);
    else XUnmapWindow(display, (Window)xWindow);

    XFlush(display);

    config.visible = visible;
}

void X11Window::SetResizable(bool resizable)
{
    Display* display = AsDisplay(xDisplay);
    if (!display || !xWindow) return;

    XSizeHints hints{};

    if (!resizable)
    {
        hints.flags = PMinSize | PMaxSize;
        hints.min_width = hints.max_width = config.size.width;
        hints.min_height = hints.max_height = config.size.height;
    }

    XSetWMNormalHints(display, (Window)xWindow, &hints);
    XFlush(display);

    config.resizable = resizable;
}

void X11Window::PollEvents()
{
    Display* display = AsDisplay(xDisplay);
    if (!display) return;

    while (XPending(display) > 0)
    {
        XEvent event{};
        XNextEvent(display, &event);

        if (inputFeed) X11Input::ProcessEvent(*inputFeed, event);

        switch (event.type)
        {
        case ClientMessage:
        {
            if (event.xclient.message_type == (Atom)wmProtocolsAtom &&
                (Atom)event.xclient.data.l[0] == (Atom)wmDeleteAtom) DispatchEvent(WindowEventType::CLOSING);

            break;
        }
        case ConfigureNotify:
        {
            const Size2i newSize(event.xconfigure.width, event.xconfigure.height);

            if (newSize != config.size)
            {
                config.size = newSize;
                DispatchEvent(WindowEventType::RESIZED);
            }

            break;
        }
        case PropertyNotify:
        {
            if (event.xproperty.atom == (Atom)netWMStateAtom) UpdateWindowState();

            break;
        }
        case FocusIn:
        {
            DispatchEvent(WindowEventType::FOCUS_GAINED);

            break;
        }
        case FocusOut:
        {
            DispatchEvent(WindowEventType::FOCUS_LOST);

            break;
        }
        default: break;
        }
    }
}

NativeWindowHandle X11Window::GetNativeHandle() const
{
    NativeWindowHandle handle{};
    handle.system = NativeWindowSystem::X11;
    handle.display = xDisplay;
    handle.window = xWindow;

    return handle;
}

void X11Window::DispatchEvent(WindowEventType type)
{
    WindowEvent event{};
    event.eventType = type;
    event.windowConfig = config;

    for (const Subscriber& subscriber : subscribers)
    {
        if (subscriber.callback) subscriber.callback(event, subscriber.user);
    }
}

void X11Window::SendNetWMState(int32 action, uint64 firstState, uint64 secondState)
{
    Display* display = AsDisplay(xDisplay);
    if (!display || !xWindow) return;

    XEvent event{};
    event.xclient.type = ClientMessage;
    event.xclient.window = (Window)xWindow;
    event.xclient.message_type = (Atom)netWMStateAtom;
    event.xclient.format = 32;

    event.xclient.data.l[0] = action;
    event.xclient.data.l[1] = (long)firstState;
    event.xclient.data.l[2] = (long)secondState;
    event.xclient.data.l[3] = 1;
    event.xclient.data.l[4] = 0;

    XSendEvent(display, DefaultRootWindow(display), False, SubstructureRedirectMask | SubstructureNotifyMask, &event);

    XFlush(display);
}

void X11Window::UpdateWindowState()
{
    Display* display = AsDisplay(xDisplay);
    if (!display || !xWindow) return;

    Atom actualType{};
    int actualFormat = 0;
    unsigned long itemCount = 0;
    unsigned long bytesAfter = 0;
    unsigned char* data = nullptr;

    bool minimized = false;
    bool maximizedHorz = false;
    bool maximizedVert = false;

    if (XGetWindowProperty(display, (Window)xWindow, (Atom)netWMStateAtom, 0, 1024, False,
        XA_ATOM, &actualType, &actualFormat, &itemCount, &bytesAfter, &data) == Success)
    {
        if (actualType == XA_ATOM && actualFormat == 32 && data)
        {
            const Atom* states = reinterpret_cast<const Atom*>(data);

            for (unsigned long i = 0; i < itemCount; ++i)
            {
                if (states[i] == (Atom)netWMStateHiddenAtom) minimized = true;
                if (states[i] == (Atom)netWMStateMaximizedHorzAtom) maximizedHorz = true;
                if (states[i] == (Atom)netWMStateMaximizedVertAtom) maximizedVert = true;
            }
        }

        if (data) XFree(data);
    }

    WindowState newState = WindowState::NORMAL;

    if (minimized) newState = WindowState::MINIMIZED;
    else if (maximizedHorz && maximizedVert) newState = WindowState::MAXIMIZED;

    if (newState == config.state) return;

    const WindowState previousState = config.state;
    config.state = newState;

    if (newState == WindowState::MINIMIZED) DispatchEvent(WindowEventType::MINIMIZED);
    else if (newState == WindowState::MAXIMIZED) DispatchEvent(WindowEventType::MAXIMIZED);
    else if (newState == WindowState::NORMAL && previousState != WindowState::NORMAL) DispatchEvent(WindowEventType::RESTORED);
}
#endif
