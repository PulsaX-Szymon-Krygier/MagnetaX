// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Graphics/GraphicsConfig.h>
#include <MX/Graphics/Resources/ImageFormat.h>
#include <Graphics/Vulkan/VulkanCommon.h>
#include "VulkanImage.h"

class VulkanTexture
{
public:
    VulkanTexture() = default;
    VulkanTexture(const VulkanTexture&) = delete;
    VulkanTexture& operator=(const VulkanTexture&) = delete;

    bool Create(VulkanDevice* _device, const uint8* pixels, uint32 width, uint32 height, ImageFormat format, const TextureConfig& config);
    void Destroy();

    VkImageView GetImageView() const { return image.GetImageView(); }
    VkSampler GetSampler() const { return sampler; }

private:
    VkDevice device = VK_NULL_HANDLE;

    VulkanImage image;

    VkSampler sampler = VK_NULL_HANDLE;

    bool UploadPixels(VulkanDevice* _device, VkBuffer stagingBuffer, uint32 width, uint32 height, uint32 mipLevels);
};
