// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "CocoaWindow.h"

#if MX_PLATFORM_APPLE
#include <Input/Cocoa/CocoaInput.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#include <cmath>
#include <string>

@interface MXCocoaWindowDelegate : NSObject <NSWindowDelegate>

@property(nonatomic, copy) void (^closeHandler)(void);

@end

@implementation MXCocoaWindowDelegate

- (BOOL)windowShouldClose:(NSWindow*)sender
{
    (void)sender;

    if (self.closeHandler) self.closeHandler();

    return NO;
}

@end

@interface MXCocoaView : NSView
@end

@implementation MXCocoaView

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)keyDown:(NSEvent*)event
{
    (void)event;
}

- (void)keyUp:(NSEvent*)event
{
    (void)event;
}

@end

struct CocoaWindow::CocoaWindowImpl
{
    NSWindow* window = nil;
    MXCocoaView* view = nil;
    CAMetalLayer* metalLayer = nil;
    MXCocoaWindowDelegate* delegate = nil;

    bool focused = false;
};

namespace
{
    NSString* UTF8ToNSString(std::string_view string)
    {
        if (string.empty()) return @"";

        return [[NSString alloc] initWithBytes:string.data() length:string.size() encoding:NSUTF8StringEncoding];
    }

    void UpdateMetalLayer(NSWindow* window, NSView* view, CAMetalLayer* metalLayer)
    {
        if (!window || !view || !metalLayer) return;

        const CGFloat scale = window.backingScaleFactor;
        const NSSize size = view.bounds.size;

        metalLayer.contentsScale = scale;
        metalLayer.drawableSize = CGSizeMake(size.width * scale, size.height * scale);
    }
}

CocoaWindow::CocoaWindow() : _impl(std::make_unique<CocoaWindowImpl>()) {}

CocoaWindow::~CocoaWindow()
{
    Destroy();
}

bool CocoaWindow::Create(const WindowConfig& createInfo)
{
    if (!_impl || _impl->window) return false;
    if (createInfo.size.width == 0 || createInfo.size.height == 0) return false;
    if (![NSThread isMainThread]) return false;

    NSApplication* application = [NSApplication sharedApplication];
    [application setActivationPolicy:NSApplicationActivationPolicyRegular];
    [application finishLaunching];

    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
    if (createInfo.resizable) style |= NSWindowStyleMaskResizable;

    const NSRect contentRect = NSMakeRect(0.0, 0.0, static_cast<CGFloat>(createInfo.size.width), static_cast<CGFloat>(createInfo.size.height));

    _impl->window = [[NSWindow alloc] initWithContentRect:contentRect styleMask:style backing:NSBackingStoreBuffered defer:NO];
    if (!_impl->window) return false;

    [_impl->window setAcceptsMouseMovedEvents:YES];

    _impl->view = [[MXCocoaView alloc] initWithFrame:contentRect];
    _impl->metalLayer = [CAMetalLayer layer];

    [_impl->window setBackgroundColor:[NSColor blackColor]];

    _impl->metalLayer.backgroundColor = [NSColor blackColor].CGColor;
    _impl->metalLayer.opaque = YES;

    [_impl->view setWantsLayer:YES];
    [_impl->view setLayer:_impl->metalLayer];
    _impl->metalLayer.delegate = (id<CALayerDelegate>)_impl->view;
    [_impl->window setContentView:_impl->view];
    [_impl->window makeFirstResponder:_impl->view];

    UpdateMetalLayer(_impl->window, _impl->view, _impl->metalLayer);

    _impl->delegate = [[MXCocoaWindowDelegate alloc] init];
    _impl->delegate.closeHandler = ^
    {
        DispatchEvent(WindowEventType::CLOSING);
    };

    [_impl->window setDelegate:_impl->delegate];
    [_impl->window center];

    config = createInfo;
    config.visible = false;

    SetResizable(createInfo.resizable);
    SetTitle(createInfo.title);

    if (createInfo.visible) SetVisibility(true);

    _impl->focused = [_impl->window isKeyWindow];

    return true;
}

void CocoaWindow::Destroy()
{
    if (!_impl || !_impl->window) return;

    [_impl->window setDelegate:nil];

    _impl->delegate.closeHandler = nil;
    _impl->delegate = nil;

    [_impl->window orderOut:nil];
    [_impl->window close];

    _impl->metalLayer = nil;
    _impl->view = nil;
    _impl->window = nil;
    _impl->focused = false;

    config.visible = false;
}

void CocoaWindow::SetSize(const Size2i& size)
{
    if (!_impl || !_impl->window) return;
    if (size.width == 0 || size.height == 0) return;

    [_impl->window setContentSize:NSMakeSize(static_cast<CGFloat>(size.width), static_cast<CGFloat>(size.height))];
    UpdateMetalLayer(_impl->window, _impl->view, _impl->metalLayer);
}

void CocoaWindow::SetTitle(std::string_view title)
{
    if (!_impl || !_impl->window) return;

    NSString* titleNS = UTF8ToNSString(title);
    if (!titleNS) return;

    [_impl->window setTitle:titleNS];

    config.title = std::string(title);
}

void CocoaWindow::SetState(const WindowState& state)
{
    if (!_impl || !_impl->window) return;

    if (!config.visible)
    {
        config.state = state;
        return;
    }

    switch (state)
    {
    case WindowState::MINIMIZED:
    {
        [_impl->window miniaturize:nil];

        break;
    }
    case WindowState::MAXIMIZED:
    {
        if ([_impl->window isMiniaturized]) [_impl->window deminiaturize:nil];
        if (![_impl->window isZoomed]) [_impl->window zoom:nil];

        break;
    }
    case WindowState::NORMAL:
    {
        if ([_impl->window isMiniaturized]) [_impl->window deminiaturize:nil];
        if ([_impl->window isZoomed]) [_impl->window zoom:nil];

        break;
    }
    }
}

void CocoaWindow::SetVisibility(bool visible)
{
    if (!_impl || !_impl->window) return;

    const WindowState state = config.state;

    if (visible)
    {
        [_impl->window makeKeyAndOrderFront:nil];
        [NSApp activateIgnoringOtherApps:YES];
    }
    else
    {
        [_impl->window orderOut:nil];
    }

    config.visible = visible;

    if (visible) SetState(state);
}

void CocoaWindow::SetResizable(bool resizable)
{
    if (!_impl || !_impl->window) return;

    NSUInteger style = [_impl->window styleMask];

    if (resizable) style |= NSWindowStyleMaskResizable;
    else style &= ~NSWindowStyleMaskResizable;

    [_impl->window setStyleMask:style];

    config.resizable = resizable;
}

void CocoaWindow::PollEvents()
{
    if (!_impl || !_impl->window) return;

    const Size2i previousSize = config.size;
    const WindowState previousState = config.state;
    const bool previousFocus = _impl->focused;
    const Size2i previousSurfaceSize = GetSurfaceSize();

    @autoreleasepool
    {
        while (true)
        {
            NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
            if (!event) break;

            if (inputFeed) CocoaInput::ProcessEvent(*inputFeed, event, _impl->view);

            [NSApp sendEvent:event];
        }
    }

    [NSApp updateWindows];

    const NSSize contentSize = _impl->view.bounds.size;
    const Size2i newSize(static_cast<int32>(std::lround(contentSize.width)), static_cast<int32>(std::lround(contentSize.height)));

    WindowState newState = config.state;

    if (config.visible)
    {
        newState = WindowState::NORMAL;

        if ([_impl->window isMiniaturized]) newState = WindowState::MINIMIZED;
        else if ([_impl->window isZoomed]) newState = WindowState::MAXIMIZED;
    }

    const bool focused = [_impl->window isKeyWindow];

    config.size = newSize;
    config.state = newState;
    _impl->focused = focused;

    UpdateMetalLayer(_impl->window, _impl->view, _impl->metalLayer);

    const Size2i newSurfaceSize = GetSurfaceSize();

    if (newState != WindowState::MINIMIZED && (newSize != previousSize || newSurfaceSize != previousSurfaceSize)) DispatchEvent(WindowEventType::RESIZED);

    if (newState != previousState)
    {
        if (newState == WindowState::MINIMIZED) DispatchEvent(WindowEventType::MINIMIZED);
        else if (newState == WindowState::MAXIMIZED) DispatchEvent(WindowEventType::MAXIMIZED);
        else DispatchEvent(WindowEventType::RESTORED);
    }

    if (focused != previousFocus)
    {
        if (focused) DispatchEvent(WindowEventType::FOCUS_GAINED);
        else DispatchEvent(WindowEventType::FOCUS_LOST);
    }
}

NativeWindowHandle CocoaWindow::GetNativeHandle() const
{
    NativeWindowHandle nativeHandle{};
    nativeHandle.system = NativeWindowSystem::COCOA;

    if (!_impl) return nativeHandle;

    nativeHandle.display = (__bridge void*)_impl->window;
    nativeHandle.window = (__bridge void*)_impl->view;

    return nativeHandle;
}

Size2i CocoaWindow::GetSurfaceSize() const
{
    if (!_impl || !_impl->metalLayer) return GetSize();

    const CGSize drawableSize = _impl->metalLayer.drawableSize;

    return Size2i(static_cast<int32>(std::lround(drawableSize.width)), static_cast<int32>(std::lround(drawableSize.height)));
}
#endif
