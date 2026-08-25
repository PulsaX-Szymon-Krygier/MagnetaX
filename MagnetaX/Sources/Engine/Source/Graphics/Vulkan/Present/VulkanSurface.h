// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Platform/NativeWindowHandle.h>
#include "../VulkanCommon.h"

class VulkanSurface
{
public:
    VulkanSurface() = default;
    VulkanSurface(const VulkanSurface&) = delete;
    VulkanSurface& operator=(const VulkanSurface&) = delete;

    bool Create(VkInstance _instance, const NativeWindowHandle& nativeWindowHandle);
    void Destroy();

    VkSurfaceKHR GetSurface() const { return surface; }

private:
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
};
