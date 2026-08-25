// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Graphics/Renderer/Deferred/GBufferDebugView.h>
#include "VulkanGBufferBindings.h"
#include "../VulkanPass.h"
#include "../VulkanPipeline.h"

class VulkanGBuffer;

struct VulkanGBufferDebugPassCreateInfo : VulkanPassCreateInfo
{
    VulkanGBuffer* gBuffer = nullptr;
    VkFormat outFormat = VK_FORMAT_UNDEFINED;
};

struct VulkanGBufferDebugPassRenderInfo : VulkanPassRenderInfo
{
    VkImageView targetView = VK_NULL_HANDLE;
    VkExtent2D extent{};
    GBufferDebugView debugView = GBufferDebugView::ALBEDO;
};

class VulkanGBufferDebugPass
{
public:
    VulkanGBufferDebugPass() = default;
    VulkanGBufferDebugPass(const VulkanGBufferDebugPass&) = delete;
    VulkanGBufferDebugPass& operator=(const VulkanGBufferDebugPass&) = delete;

    bool Create(const VulkanGBufferDebugPassCreateInfo& createInfo);
    void Destroy();

    void Record(const VulkanGBufferDebugPassRenderInfo& renderInfo);

private:
    VulkanPipeline pipeline;
    VulkanGBufferBindings gBufferBindings;
};
