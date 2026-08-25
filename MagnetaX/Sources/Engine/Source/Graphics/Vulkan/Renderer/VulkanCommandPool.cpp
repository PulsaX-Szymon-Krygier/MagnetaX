// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanCommandPool.h"

#if MX_GRAPHICS_VULKAN
#include "../VulkanInitializers.h"

bool VulkanCommandPool::Create(VkDevice _device, uint32 queueFamilyIndex)
{
    if (!_device || queueFamilyIndex == UINT32_MAX) return false;

    Destroy();

    device = _device;

    const VkCommandPoolCreateInfo poolInfo = VulkanInitializers::CommandPoolCreateInfo(
        queueFamilyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
    );

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    return true;
}

void VulkanCommandPool::Destroy()
{
    if (device && commandPool) vkDestroyCommandPool(device, commandPool, nullptr);

    commandPool = VK_NULL_HANDLE;
    device = VK_NULL_HANDLE;
}

VkCommandBuffer VulkanCommandPool::AllocateCommandBuffer() const
{
    if (!device || !commandPool) return VK_NULL_HANDLE;

    const VkCommandBufferAllocateInfo allocInfo = VulkanInitializers::CommandBufferAllocateInfo(
        commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1
    );

    VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;

    if (vkAllocateCommandBuffers(device, &allocInfo, &cmdBuffer) != VK_SUCCESS) return VK_NULL_HANDLE;

    return cmdBuffer;
}
#endif
