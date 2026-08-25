// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "../VulkanSurface.h"

#if MX_GRAPHICS_VULKAN && MX_PLATFORM_APPLE
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

bool VulkanSurface::Create(VkInstance _instance, const NativeWindowHandle& nativeWindowHandle)
{
    if (!_instance) return false;
    if (nativeWindowHandle.system != NativeWindowSystem::COCOA) return false;
    if (!nativeWindowHandle.window) return false;

    NSView* view = (__bridge NSView*)nativeWindowHandle.window;
    CALayer* layer = view.layer;

    if (!layer || ![layer isKindOfClass:[CAMetalLayer class]]) return false;

    Destroy();

    instance = _instance;

    VkMetalSurfaceCreateInfoEXT surfaceInfo{};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    surfaceInfo.pLayer = (CAMetalLayer*)layer;
    surfaceInfo.pNext = nullptr;

    if (vkCreateMetalSurfaceEXT(instance, &surfaceInfo, nullptr, &surface) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    return true;
}
#endif
