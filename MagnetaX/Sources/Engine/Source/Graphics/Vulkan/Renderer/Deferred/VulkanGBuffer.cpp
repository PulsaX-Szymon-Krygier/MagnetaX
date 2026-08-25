// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanGBuffer.h"

#if MX_GRAPHICS_VULKAN
bool VulkanGBuffer::Create(VulkanDevice* _device, VkExtent2D _extent)
{
    if (!_device || _extent.width == 0 || _extent.height == 0) return false;

    Destroy();

    extent = _extent;

    const VkImageUsageFlags colorUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    const VkImageUsageFlags depthUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    if (!albedoImage.Create(_device, extent, ImageFormat::RGBA8_UNORM, colorUsage))
    {
        Destroy();
        return false;
    }

    if (!normalImage.Create(_device, extent, ImageFormat::RGBA16_FLOAT, colorUsage))
    {
        Destroy();
        return false;
    }

    if (!materialImage.Create(_device, extent, ImageFormat::RGBA8_UNORM, colorUsage))
    {
        Destroy();
        return false;
    }

    if (!depthImage.Create(_device, extent, ImageFormat::D32_FLOAT, depthUsage))
    {
        Destroy();
        return false;
    }

    return true;
}

void VulkanGBuffer::Destroy()
{
    depthImage.Destroy();
    materialImage.Destroy();
    normalImage.Destroy();
    albedoImage.Destroy();

    extent = {};
}
#endif
