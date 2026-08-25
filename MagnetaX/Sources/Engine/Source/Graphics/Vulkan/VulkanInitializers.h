// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "VulkanMinimal.h"

struct VulkanInitializers
{
    static VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32 queueFamilyIndex, VkCommandPoolCreateFlags flags);
    static VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool commandPool, VkCommandBufferLevel level, uint32 cmdBufferCount);
    static VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);

    static VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(uint32 bindingCount, const VkDescriptorSetLayoutBinding* bindings);
    static VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo(uint32 maxSets, uint32 poolSizeCount, const VkDescriptorPoolSize* poolSizes);
    static VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo(VkDescriptorPool descPool,
        uint32 descSetCount, const VkDescriptorSetLayout* descSetLayouts);

    static VkImageMemoryBarrier2 ImageMemoryBarrier(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldLayout, VkImageLayout newLayout,
        VkPipelineStageFlags2 srcStageMask, VkAccessFlags2 srcAccessMask, VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask,
        uint32 baseMipLevel = 0, uint32 levelCount = 1, uint32 baseArrayLayer = 0, uint32 layerCount = 1);
};
