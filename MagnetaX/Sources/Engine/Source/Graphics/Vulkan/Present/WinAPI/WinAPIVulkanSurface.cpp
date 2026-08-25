// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "../VulkanSurface.h"

#if MX_GRAPHICS_VULKAN && MX_PLATFORM_WINDOWS
#include <Windows.h>
#include <vulkan/vulkan_win32.h>

bool VulkanSurface::Create(VkInstance _instance, const NativeWindowHandle& nativeWindowHandle)
{
    if (!_instance) return false;
    if (nativeWindowHandle.system != NativeWindowSystem::WIN_API) return false;
    if (!nativeWindowHandle.display || !nativeWindowHandle.window) return false;

    Destroy();

    instance = _instance;

    VkWin32SurfaceCreateInfoKHR surfaceInfo{};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.hinstance = (HINSTANCE)nativeWindowHandle.display;
    surfaceInfo.hwnd = (HWND)nativeWindowHandle.window;
    surfaceInfo.pNext = nullptr;

    if (vkCreateWin32SurfaceKHR(instance, &surfaceInfo, nullptr, &surface) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    return true;
}
#endif
