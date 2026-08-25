// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanImageFormat.h"

#if MX_GRAPHICS_VULKAN
VkFormat VulkanImageFormat::FromImageFormat(ImageFormat format)
{
    switch (format)
    {
    case ImageFormat::RGBA8_SRGB:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case ImageFormat::BGRA8_SRGB:
        return VK_FORMAT_B8G8R8A8_SRGB;
    case ImageFormat::RGBA8_UNORM:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case ImageFormat::RGBA16_FLOAT:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case ImageFormat::D32_FLOAT:
        return VK_FORMAT_D32_SFLOAT;
    case ImageFormat::R8_UNORM:
        return VK_FORMAT_R8_UNORM;
    default:
        return VK_FORMAT_UNDEFINED;
    }
}

VkImageAspectFlags VulkanImageFormat::GetImageAspect(ImageFormat format)
{
    switch (format)
    {
    case ImageFormat::RGBA8_SRGB:
    case ImageFormat::BGRA8_SRGB:
    case ImageFormat::RGBA8_UNORM:
    case ImageFormat::RGBA16_FLOAT:
    case ImageFormat::R8_UNORM:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    case ImageFormat::D32_FLOAT:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    default:
        return 0;
    }
}
#endif
