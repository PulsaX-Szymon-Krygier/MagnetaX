// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanSurface.h"

#if MX_GRAPHICS_VULKAN
void VulkanSurface::Destroy()
{
    if (instance && surface) vkDestroySurfaceKHR(instance, surface, nullptr);

    surface = VK_NULL_HANDLE;
    instance = VK_NULL_HANDLE;
}
#endif
