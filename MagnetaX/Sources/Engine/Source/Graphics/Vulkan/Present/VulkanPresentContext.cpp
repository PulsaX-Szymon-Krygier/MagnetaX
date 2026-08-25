// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanPresentContext.h"

#if MX_GRAPHICS_VULKAN
bool VulkanPresentContext::Create(const VulkanPresentContextCreateInfo& createInfo)
{
    if (!createInfo.device || !createInfo.surface) return false;
    if (createInfo.extent.width == 0 || createInfo.extent.height == 0) return false;

    Destroy();

    VulkanSwapchainCreateInfo swapchainInfo{};
    swapchainInfo.device = createInfo.device;
    swapchainInfo.surface = createInfo.surface;
    swapchainInfo.extent = createInfo.extent;

    if (!swapchain.Create(swapchainInfo))
    {
        Destroy();
        return false;
    }

    return true;
}

void VulkanPresentContext::Destroy()
{
    swapchain.Destroy();
}
#endif
