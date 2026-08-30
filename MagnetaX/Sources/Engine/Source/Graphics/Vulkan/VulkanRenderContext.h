// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Graphics/Renderer/RendererConfig.h>
#include "Present/VulkanPresentContext.h"
#include "Renderer/VulkanRenderer.h"

class VulkanUIRenderer;

struct VulkanRenderContextCreateInfo
{
    VulkanDevice* device = nullptr;

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkExtent2D extent{};

    VkDescriptorSetLayout materialDescSetLayout = VK_NULL_HANDLE;

    RendererConfig config{};

    VulkanUIRenderer* uiRenderer = nullptr;
};

class VulkanRenderContext
{
public:
    VulkanRenderContext() = default;
    VulkanRenderContext(const VulkanRenderContext&) = delete;
    VulkanRenderContext& operator=(const VulkanRenderContext&) = delete;

    bool Create(const VulkanRenderContextCreateInfo& createInfo);
    void Destroy();

    bool IsValid() const { return presentContext.IsValid(); }

    VulkanRenderer& GetRenderer() { return renderer; }
    const VulkanRenderer& GetRenderer() const { return renderer; }

    VulkanPresentContext& GetPresentContext() { return presentContext; }
    const VulkanPresentContext& GetPresentContext() const { return presentContext; }

private:
    VulkanPresentContext presentContext;
    VulkanRenderer renderer;
};
