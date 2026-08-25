// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "VulkanSwapchain.h"

struct VulkanPresentContextCreateInfo
{
    VulkanDevice* device = nullptr;

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkExtent2D extent{};
};

class VulkanPresentContext
{
public:
    VulkanPresentContext() = default;
    VulkanPresentContext(const VulkanPresentContext&) = delete;
    VulkanPresentContext& operator=(const VulkanPresentContext&) = delete;

    bool Create(const VulkanPresentContextCreateInfo& createInfo);
    void Destroy();

    bool IsValid() const { return swapchain.GetSwapchain() != VK_NULL_HANDLE; }

    VulkanSwapchain& GetSwapchain() { return swapchain; }
    const VulkanSwapchain& GetSwapchain() const { return swapchain; }

private:
    VulkanSwapchain swapchain;
};
