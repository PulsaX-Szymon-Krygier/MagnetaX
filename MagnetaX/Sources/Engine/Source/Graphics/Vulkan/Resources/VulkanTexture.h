// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Graphics/GraphicsConfig.h>
#include <MX/Graphics/Resources/ImageFormat.h>
#include <Graphics/Vulkan/VulkanCommon.h>
#include "VulkanImage.h"

struct VulkanTextureCreateInfo
{
    VulkanDevice* device = nullptr;

    const uint8* pixels = nullptr;
    uint32 width = 0;
    uint32 height = 0;
    ImageFormat format = ImageFormat::UNKNOWN;

    TextureConfig config{};

    VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
};

class VulkanTexture
{
public:
    VulkanTexture() = default;
    VulkanTexture(const VulkanTexture&) = delete;
    VulkanTexture& operator=(const VulkanTexture&) = delete;

    bool Create(const VulkanTextureCreateInfo& createInfo);
    void Destroy();

    VkImageView GetImageView() const { return image.GetImageView(); }
    VkSampler GetSampler() const { return sampler; }

private:
    VkDevice device = VK_NULL_HANDLE;

    VulkanImage image;

    VkSampler sampler = VK_NULL_HANDLE;

    bool UploadPixels(VulkanDevice* _device, VkBuffer stagingBuffer, uint32 width, uint32 height, uint32 mipLevels);
};
