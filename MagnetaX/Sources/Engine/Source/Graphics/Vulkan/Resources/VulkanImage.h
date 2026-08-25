// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Graphics/Vulkan/VulkanCommon.h>
#include "VulkanImageFormat.h"

class VulkanImage
{
public:
    VulkanImage() = default;
    VulkanImage(const VulkanImage&) = delete;
    VulkanImage& operator=(const VulkanImage&) = delete;

    VulkanImage(VulkanImage&& other) noexcept;
    VulkanImage& operator=(VulkanImage&& other) noexcept;

    bool Create(VkDevice _device, VkImage _image, VkFormat _format, VkImageAspectFlags aspect);
    bool Create(VulkanDevice* _device, VkExtent2D extent, ImageFormat _format, VkImageUsageFlags usage,
        uint32 mipLevels = 1, uint32 arrayLayers = 1);

    void Destroy();

    VkImage GetImage() const { return image; }
    VkImageView GetImageView() const { return imageView; }
    VkFormat GetFormat() const { return format; }

private:
    VkDevice device = VK_NULL_HANDLE;

    VkImage image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkDeviceMemory memory = VK_NULL_HANDLE;

    bool ownsImage = false;

    bool CreateImageView(VkImageAspectFlags aspect, uint32 mipLevels = 1, uint32 arrayLayers = 1);
};
