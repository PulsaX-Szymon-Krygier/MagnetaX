// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "../VulkanPass.h"
#include "../VulkanPipeline.h"
#include <span>

struct VulkanEquirectPassCreateInfo : VulkanPassCreateInfo
{
    VkFormat outFormat = VK_FORMAT_UNDEFINED;
};

struct VulkanEquirectPassRenderInfo : VulkanPassRenderInfo
{
    VulkanTexture* sourceTexture = nullptr;

    VkImage targetImage = VK_NULL_HANDLE;
    std::span<const VkImageView> targetViews;
    VkExtent2D extent{};
};

class VulkanEquirectPass
{
public:
    VulkanEquirectPass() = default;

    bool Create(const VulkanEquirectPassCreateInfo& createInfo);
    void Destroy();

    void Record(const VulkanEquirectPassRenderInfo& renderInfo);

private:
    VkDevice device = VK_NULL_HANDLE;

    VulkanPipeline pipeline;

    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
    VkDescriptorSet descSet = VK_NULL_HANDLE;
};
