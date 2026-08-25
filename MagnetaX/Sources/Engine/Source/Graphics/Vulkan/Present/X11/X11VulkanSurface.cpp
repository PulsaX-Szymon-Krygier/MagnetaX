// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "../VulkanSurface.h"

#if MX_GRAPHICS_VULKAN && MX_PLATFORM_X11
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>

bool VulkanSurface::Create(VkInstance _instance, const NativeWindowHandle& nativeWindowHandle)
{
    if (!_instance) return false;
    if (nativeWindowHandle.system != NativeWindowSystem::X11) return false;
    if (!nativeWindowHandle.display || !nativeWindowHandle.window) return false;

    Destroy();

    instance = _instance;

    VkXlibSurfaceCreateInfoKHR surfaceInfo{};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.dpy = (Display*)nativeWindowHandle.display;
    surfaceInfo.window = (Window)nativeWindowHandle.window;
    surfaceInfo.pNext = nullptr;

    if (vkCreateXlibSurfaceKHR(instance, &surfaceInfo, nullptr, &surface) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    return true;
}
#endif
