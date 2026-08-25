// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanInitializers.h"

#if MX_GRAPHICS_VULKAN
VkCommandPoolCreateInfo VulkanInitializers::CommandPoolCreateInfo(uint32 queueFamilyIndex, VkCommandPoolCreateFlags flags)
{
    VkCommandPoolCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.queueFamilyIndex = queueFamilyIndex;
    info.flags = flags;
    info.pNext = nullptr;

    return info;
}

VkCommandBufferAllocateInfo VulkanInitializers::CommandBufferAllocateInfo(VkCommandPool commandPool, VkCommandBufferLevel level, uint32 cmdBufferCount)
{
    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = commandPool;
    info.level = level;
    info.commandBufferCount = cmdBufferCount;
    info.pNext = nullptr;

    return info;
}

VkCommandBufferBeginInfo VulkanInitializers::CommandBufferBeginInfo(VkCommandBufferUsageFlags flags)
{
    VkCommandBufferBeginInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = flags;
    info.pNext = nullptr;

    return info;
}

VkDescriptorSetLayoutCreateInfo VulkanInitializers::DescriptorSetLayoutCreateInfo(uint32 bindingCount,
    const VkDescriptorSetLayoutBinding* bindings)
{
    VkDescriptorSetLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = bindingCount;
    info.pBindings = bindings;
    info.pNext = nullptr;

    return info;
}

VkDescriptorPoolCreateInfo VulkanInitializers::DescriptorPoolCreateInfo(uint32 maxSets, uint32 poolSizeCount,
    const VkDescriptorPoolSize* poolSizes)
{
    VkDescriptorPoolCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.maxSets = maxSets;
    info.poolSizeCount = poolSizeCount;
    info.pPoolSizes = poolSizes;
    info.pNext = nullptr;

    return info;
}

VkDescriptorSetAllocateInfo VulkanInitializers::DescriptorSetAllocateInfo(VkDescriptorPool descPool,
    uint32 descSetCount, const VkDescriptorSetLayout* descSetLayouts)
{
    VkDescriptorSetAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.descriptorPool = descPool;
    info.descriptorSetCount = descSetCount;
    info.pSetLayouts = descSetLayouts;
    info.pNext = nullptr;

    return info;
}

VkImageMemoryBarrier2 VulkanInitializers::ImageMemoryBarrier(VkImage image, VkImageAspectFlags aspectMask,
    VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags2 srcStageMask, VkAccessFlags2 srcAccessMask,
    VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask, uint32 baseMipLevel, uint32 levelCount,
    uint32 baseArrayLayer, uint32 layerCount)
{
    VkImageMemoryBarrier2 barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    barrier.srcStageMask = srcStageMask;
    barrier.srcAccessMask = srcAccessMask;
    barrier.dstStageMask = dstStageMask;
    barrier.dstAccessMask = dstAccessMask;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspectMask;
    barrier.subresourceRange.baseMipLevel = baseMipLevel;
    barrier.subresourceRange.levelCount = levelCount;
    barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
    barrier.subresourceRange.layerCount = layerCount;
    barrier.pNext = nullptr;

    return barrier;
}
#endif
