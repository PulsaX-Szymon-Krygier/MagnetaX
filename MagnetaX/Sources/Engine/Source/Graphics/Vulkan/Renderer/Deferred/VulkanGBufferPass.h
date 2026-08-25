// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "VulkanGBuffer.h"
#include "../VulkanDrawItem.h"
#include "../VulkanPass.h"
#include "../VulkanPipeline.h"
#include <span>

struct VulkanGBufferPassCreateInfo : VulkanPassCreateInfo
{
    VkExtent2D extent{};
    VkDescriptorSetLayout materialDescSetLayout = VK_NULL_HANDLE;
};

struct VulkanGBufferPassRenderInfo : VulkanPassRenderInfo
{
    std::span<const VulkanDrawItem> drawItems;
};

class VulkanGBufferPass
{
public:
    VulkanGBufferPass() = default;
    VulkanGBufferPass(const VulkanGBufferPass&) = delete;
    VulkanGBufferPass& operator=(const VulkanGBufferPass&) = delete;

    bool Create(const VulkanGBufferPassCreateInfo& createInfo);
    void Destroy();

    void Record(const VulkanGBufferPassRenderInfo& renderInfo);

    VulkanGBuffer& GetGBuffer() { return gBuffer; }
    const VulkanGBuffer& GetGBuffer() const { return gBuffer; }

private:
    VulkanGBuffer gBuffer;
    VulkanPipeline pipeline;
};
