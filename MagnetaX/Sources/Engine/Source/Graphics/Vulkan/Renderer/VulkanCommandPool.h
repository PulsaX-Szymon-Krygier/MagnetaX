// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "../VulkanMinimal.h"

class VulkanCommandPool
{
public:
    VulkanCommandPool() = default;
    VulkanCommandPool(const VulkanCommandPool&) = delete;
    VulkanCommandPool& operator=(const VulkanCommandPool&) = delete;

    bool Create(VkDevice _device, uint32 queueFamilyIndex);
    void Destroy();

    VkCommandBuffer AllocateCommandBuffer() const;

    VkCommandPool GetCommandPool() const { return commandPool; }

private:
    VkDevice device = VK_NULL_HANDLE;

    VkCommandPool commandPool = VK_NULL_HANDLE;
};
