// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Graphics/Vulkan/VulkanCommon.h>

class VulkanBuffer
{
public:
    VulkanBuffer() = default;
    VulkanBuffer(const VulkanBuffer&) = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;

    bool Create(VulkanDevice* _device, VkDeviceSize _deviceSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryProps);
    void Destroy();

    bool Upload(const void* data, VkDeviceSize dataSize);

    VkBuffer GetBuffer() const { return buffer; }
    VkDeviceSize GetSize() const { return deviceSize; }

private:
    VkDevice device = VK_NULL_HANDLE;

    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDeviceSize deviceSize = 0;
};
