// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanMesh.h"

#if MX_GRAPHICS_VULKAN
bool VulkanMesh::Create(const VulkanMeshCreateInfo& createInfo)
{
    if (!createInfo.device || !createInfo.vertexData || !createInfo.indices) return false;
    if (createInfo.vertexDataSize == 0 || createInfo.indexCount == 0) return false;

    Destroy();

    const VkMemoryPropertyFlags memoryProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    if (!vertexBuffer.Create(createInfo.device, createInfo.vertexDataSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, memoryProps))
    {
        Destroy();
        return false;
    }

    if (!vertexBuffer.Upload(createInfo.vertexData, createInfo.vertexDataSize))
    {
        Destroy();
        return false;
    }

    const VkDeviceSize indexDataSize = sizeof(uint32) * createInfo.indexCount;

    if (!indexBuffer.Create(createInfo.device, indexDataSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, memoryProps))
    {
        Destroy();
        return false;
    }

    if (!indexBuffer.Upload(createInfo.indices, indexDataSize))
    {
        Destroy();
        return false;
    }

    indexCount = createInfo.indexCount;

    return true;
}

void VulkanMesh::Destroy()
{
    indexBuffer.Destroy();
    vertexBuffer.Destroy();

    indexCount = 0;
}
#endif
