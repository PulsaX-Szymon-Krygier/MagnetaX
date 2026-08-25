// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "VulkanMinimal.h"

class VulkanQueueFamily
{
public:
    struct Indices
    {
        uint32 graphics = UINT32_MAX;
        uint32 transfer = UINT32_MAX;
        uint32 present = UINT32_MAX;

        bool HasGraphics() const { return graphics != UINT32_MAX; }
        bool HasTransfer() const { return transfer != UINT32_MAX; }
        bool HasPresent() const { return present != UINT32_MAX; }

        bool IsCoreComplete() const { return HasGraphics(); }

        bool IsComplete() const { return HasGraphics() && HasPresent(); }
    };

    static Indices FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface = VK_NULL_HANDLE);
};
