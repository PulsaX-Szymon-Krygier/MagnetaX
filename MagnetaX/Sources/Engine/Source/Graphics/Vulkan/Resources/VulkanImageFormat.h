// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Graphics/Vulkan/VulkanMinimal.h>

#include <MX/Graphics/Resources/ImageFormat.h>

class VulkanImageFormat
{
public:
    static VkFormat FromImageFormat(ImageFormat format);
    static ImageFormat ToImageFormat(VkFormat format);

    static VkImageAspectFlags GetImageAspect(ImageFormat format);
};
