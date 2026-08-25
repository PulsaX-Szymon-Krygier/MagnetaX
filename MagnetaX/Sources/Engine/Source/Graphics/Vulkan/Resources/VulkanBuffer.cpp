// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanBuffer.h"

#if MX_GRAPHICS_VULKAN
#include "../VulkanDevice.h"
#include <cstring>

bool VulkanBuffer::Create(VulkanDevice* _device, VkDeviceSize _deviceSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryProps)
{
    if (!_device || _deviceSize == 0) return false;

    const VkDevice buffDevice = _device->GetDevice();
    if (!buffDevice) return false;

    Destroy();

    device = buffDevice;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = _deviceSize;
    bufferInfo.usage = usageFlags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.pNext = nullptr;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    VkMemoryRequirements memoryReqs{};
    vkGetBufferMemoryRequirements(device, buffer, &memoryReqs);

    const uint32 memoryTypeIndex = _device->FindMemoryType(memoryReqs.memoryTypeBits, memoryProps);

    if (memoryTypeIndex == UINT32_MAX)
    {
        Destroy();
        return false;
    }

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryReqs.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    allocInfo.pNext = nullptr;

    if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    if (vkBindBufferMemory(device, buffer, memory, 0) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    deviceSize = _deviceSize;

    return true;
}

void VulkanBuffer::Destroy()
{
    if (device)
    {
        if (buffer) vkDestroyBuffer(device, buffer, nullptr);
        if (memory) vkFreeMemory(device, memory, nullptr);
    }

    buffer = VK_NULL_HANDLE;
    memory = VK_NULL_HANDLE;
    deviceSize = 0;

    device = VK_NULL_HANDLE;
}

bool VulkanBuffer::Upload(const void* data, VkDeviceSize dataSize)
{
    if (!device || !memory || !data) return false;
    if (dataSize == 0 || dataSize > deviceSize) return false;

    void* mappedData = nullptr;

    if (vkMapMemory(device, memory, 0, dataSize, 0, &mappedData) != VK_SUCCESS) return false;

    std::memcpy(mappedData, data, (usize)dataSize);

    vkUnmapMemory(device, memory);

    return true;
}
#endif
