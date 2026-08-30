// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Graphics/Renderer/UI/UIDrawData.h>
#include <Graphics/Vulkan/Resources/VulkanBuffer.h>
#include "../VulkanPass.h"
#include "../VulkanPipeline.h"

class VulkanUIRenderer;

struct VulkanUIPassCreateInfo : VulkanPassCreateInfo
{
    VkFormat outFormat = VK_FORMAT_UNDEFINED;
    VulkanUIRenderer* uiRenderer = nullptr;
};

struct VulkanUIPassRenderInfo : VulkanPassRenderInfo
{
    VkImageView targetView = VK_NULL_HANDLE;
    VkExtent2D extent{};

    bool clearTarget = false;
};

class VulkanUIPass
{
public:
    bool Create(const VulkanUIPassCreateInfo& createInfo);
    void Destroy();

    void Record(const VulkanUIPassRenderInfo& renderInfo);

private:
    VulkanDevice* device = nullptr;

    VulkanPipeline pipeline;

    VulkanBuffer vertexBuffer;
    VulkanBuffer indexBuffer;

    VulkanUIRenderer* uiRenderer = nullptr;

    bool UpdateBuffers(const UIDrawData& drawData);
};
