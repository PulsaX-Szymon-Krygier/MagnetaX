// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanRenderContext.h"

#if MX_GRAPHICS_VULKAN
bool VulkanRenderContext::Create(const VulkanRenderContextCreateInfo& createInfo)
{
    if (!createInfo.device || !createInfo.surface || !createInfo.materialDescSetLayout || !createInfo.uiRenderer) return false;
    if (createInfo.extent.width == 0 || createInfo.extent.height == 0) return false;

    Destroy();

    VulkanPresentContextCreateInfo presentInfo{};
    presentInfo.device = createInfo.device;
    presentInfo.surface = createInfo.surface;
    presentInfo.extent = createInfo.extent;

    if (!presentContext.Create(presentInfo))
    {
        Destroy();
        return false;
    }

    VulkanRendererCreateInfo rendererInfo{};
    rendererInfo.device = createInfo.device;
    rendererInfo.presentContext = &presentContext;
    rendererInfo.materialDescSetLayout = createInfo.materialDescSetLayout;
    rendererInfo.config = createInfo.config;
    rendererInfo.uiRenderer = createInfo.uiRenderer;

    if (!renderer.Create(rendererInfo))
    {
        Destroy();
        return false;
    }

    return true;
}

void VulkanRenderContext::Destroy()
{
    renderer.Destroy();
    presentContext.Destroy();
}
#endif
