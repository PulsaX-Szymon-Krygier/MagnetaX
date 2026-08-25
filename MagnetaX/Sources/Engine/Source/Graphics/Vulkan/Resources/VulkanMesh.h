// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Graphics/Vulkan/VulkanCommon.h>
#include "VulkanBuffer.h"

struct VulkanMeshCreateInfo
{
    VulkanDevice* device = nullptr;

    const void* vertexData = nullptr;
    VkDeviceSize vertexDataSize = 0;

    const uint32* indices = nullptr;
    uint32 indexCount = 0;
};

class VulkanMesh
{
public:
    VulkanMesh() = default;
    VulkanMesh(const VulkanMesh&) = delete;
    VulkanMesh& operator=(const VulkanMesh&) = delete;

    bool Create(const VulkanMeshCreateInfo& createInfo);
    void Destroy();

    VkBuffer GetVertexBuffer() const { return vertexBuffer.GetBuffer(); }
    VkBuffer GetIndexBuffer() const { return indexBuffer.GetBuffer(); }

    uint32 GetIndexCount() const { return indexCount; }

private:
    VulkanBuffer vertexBuffer;
    VulkanBuffer indexBuffer;

    uint32 indexCount = 0;
};
