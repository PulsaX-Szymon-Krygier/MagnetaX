// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Graphics/Renderer/UI/UIRenderData.h>
#include <Graphics/Vulkan/Resources/VulkanBuffer.h>
#include <Graphics/Vulkan/Resources/VulkanTexture.h>
#include "../VulkanPass.h"
#include "../VulkanPipeline.h"
#include <span>

class UIFont;

struct VulkanUIPassCreateInfo : VulkanPassCreateInfo
{
    VkFormat outFormat = VK_FORMAT_UNDEFINED;
};

struct VulkanUIPassRenderInfo : VulkanPassRenderInfo
{
    VkImageView targetView = VK_NULL_HANDLE;
    VkExtent2D extent{};

    const UIRenderData* uiData = nullptr;
};

class VulkanUIPass
{
public:
    bool Create(const VulkanUIPassCreateInfo& createInfo);
    void Destroy();

    void Record(const VulkanUIPassRenderInfo& renderInfo);

private:
    VulkanDevice* device = nullptr;

    const UIFont* font = nullptr;
    uint64 fontVersion = 0;

    VulkanTexture fontAtlas;
    VulkanPipeline pipeline;
    VulkanBuffer vertexBuffer;

    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
    VkDescriptorSet descSet = VK_NULL_HANDLE;

    bool SetFont(const UIFont& _font);
    bool UpdateVertexBuffer(std::span<const UIVertex> vertices);
};
