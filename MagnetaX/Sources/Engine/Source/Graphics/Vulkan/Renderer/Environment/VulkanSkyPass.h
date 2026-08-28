// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "../VulkanPass.h"
#include "../VulkanPipeline.h"
#include "../Deferred/VulkanGBufferBindings.h"

class VulkanGBuffer;
struct RenderSceneData;

struct VulkanSkyPassCreateInfo : VulkanPassCreateInfo
{
    VulkanGBuffer* gBuffer = nullptr;
    VkFormat outFormat = VK_FORMAT_UNDEFINED;
};

struct VulkanSkyPassRenderInfo : VulkanPassRenderInfo
{
    VkImageView targetView = VK_NULL_HANDLE;
    VkExtent2D extent{};
    VkImageView environmentView = VK_NULL_HANDLE;
    VkSampler environmentSampler = VK_NULL_HANDLE;
    const RenderSceneData* sceneData = nullptr;
};

class VulkanSkyPass
{
public:
    VulkanSkyPass() = default;

    bool Create(const VulkanSkyPassCreateInfo& createInfo);
    void Destroy();
    void Record(const VulkanSkyPassRenderInfo& renderInfo);

private:
    VkDevice device = VK_NULL_HANDLE;

    VulkanPipeline pipeline;
    VulkanGBufferBindings gBufferBindings;

    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
    VkDescriptorSet descSet = VK_NULL_HANDLE;
};
