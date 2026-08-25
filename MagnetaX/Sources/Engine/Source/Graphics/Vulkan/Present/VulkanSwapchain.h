// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Graphics/Vulkan/Resources/VulkanImage.h>
#include "../VulkanCommon.h"
#include <vector>

struct VulkanSwapchainCreateInfo
{
    VulkanDevice* device = nullptr;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkExtent2D extent{};
};

class VulkanSwapchain
{
public:
    VulkanSwapchain() = default;
    VulkanSwapchain(const VulkanSwapchain&) = delete;
    VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

    bool Create(const VulkanSwapchainCreateInfo& createInfo);
    void Destroy();

    VkSwapchainKHR GetSwapchain() const { return swapchain; }
    VkFormat GetFormat() const { return format; }
    VkExtent2D GetExtent() const { return extent; }

    const std::vector<VulkanImage>& GetImages() const { return images; }

private:
    VkDevice device = VK_NULL_HANDLE;

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkExtent2D extent{};

    std::vector<VulkanImage> images;
};
