// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Graphics/Vulkan/VulkanCommon.h>

class VulkanGBuffer;

class VulkanGBufferBindings
{
public:
    VulkanGBufferBindings() = default;
    VulkanGBufferBindings(const VulkanGBufferBindings&) = delete;
    VulkanGBufferBindings& operator=(const VulkanGBufferBindings&) = delete;

    bool Create(VulkanDevice* _device, VulkanGBuffer* gBuffer);
    void Destroy();

    VkDescriptorSetLayout GetDescriptorSetLayout() const { return descSetLayout; }
    VkDescriptorSet GetDescriptorSet() const { return descSet; }

private:
    VkDevice device = VK_NULL_HANDLE;

    VkSampler sampler = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
    VkDescriptorSet descSet = VK_NULL_HANDLE;
};
