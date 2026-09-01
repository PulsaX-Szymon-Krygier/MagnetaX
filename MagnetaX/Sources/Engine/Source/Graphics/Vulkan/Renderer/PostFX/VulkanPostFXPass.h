// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Graphics/Renderer/PostFX/FXAAConfig.h>
#include "../VulkanPass.h"
#include "../VulkanPipeline.h"

struct VulkanPostFXPassCreateInfo : VulkanPassCreateInfo
{
    VulkanImage* srcImage = nullptr;
    VkFormat outFormat = VK_FORMAT_UNDEFINED;
};

struct VulkanPostFXPassRenderInfo : VulkanPassRenderInfo
{
    VkImageView targetView = VK_NULL_HANDLE;
    VkExtent2D extent{};
    FXAAConfig config{};
};

class VulkanPostFXPass
{
public:
    VulkanPostFXPass() = default;
    VulkanPostFXPass(const VulkanPostFXPass&) = delete;
    VulkanPostFXPass& operator=(const VulkanPostFXPass&) = delete;

    bool Create(const VulkanPostFXPassCreateInfo& createInfo);
    void Destroy();

    void Record(const VulkanPostFXPassRenderInfo& renderInfo);

private:
    VkDevice device = VK_NULL_HANDLE;

    VulkanPipeline pipeline;

    VkSampler sampler = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
    VkDescriptorSet descSet = VK_NULL_HANDLE;
};
