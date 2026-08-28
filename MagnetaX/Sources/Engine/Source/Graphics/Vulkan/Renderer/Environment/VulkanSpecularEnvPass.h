// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "../VulkanPass.h"
#include "../VulkanPipeline.h"
#include <span>

struct VulkanSpecularEnvPassCreateInfo : VulkanPassCreateInfo
{
    VkFormat outFormat = VK_FORMAT_UNDEFINED;
};

struct VulkanSpecularEnvPassRenderInfo : VulkanPassRenderInfo
{
    VkImageView sourceView = VK_NULL_HANDLE;
    VkSampler sourceSampler = VK_NULL_HANDLE;

    VkImage targetImage = VK_NULL_HANDLE;
    std::span<const VkImageView> targetViews;

    VkExtent2D extent{};
    uint32 mipLevels = 1;
};

class VulkanSpecularEnvPass
{
public:
    VulkanSpecularEnvPass() = default;

    bool Create(const VulkanSpecularEnvPassCreateInfo& createInfo);
    void Destroy();
    void Record(const VulkanSpecularEnvPassRenderInfo& renderInfo);

private:
    VkDevice device = VK_NULL_HANDLE;

    VulkanPipeline pipeline;

    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
    VkDescriptorSet descSet = VK_NULL_HANDLE;
};
