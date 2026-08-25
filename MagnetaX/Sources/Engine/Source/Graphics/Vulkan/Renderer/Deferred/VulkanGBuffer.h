// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Graphics/Vulkan/Resources/VulkanImage.h>

class VulkanGBuffer
{
public:
    VulkanGBuffer() = default;
    VulkanGBuffer(const VulkanGBuffer&) = delete;
    VulkanGBuffer& operator=(const VulkanGBuffer&) = delete;

    bool Create(VulkanDevice* _device, VkExtent2D _extent);
    void Destroy();

    const VulkanImage& GetAlbedoImage() const { return albedoImage; }
    const VulkanImage& GetNormalImage() const { return normalImage; }
    const VulkanImage& GetMaterialImage() const { return materialImage; }
    const VulkanImage& GetDepthImage() const { return depthImage; }

    VkExtent2D GetExtent() const { return extent; }

private:
    VulkanImage albedoImage;
    VulkanImage normalImage;
    VulkanImage materialImage;
    VulkanImage depthImage;

    VkExtent2D extent{};
};
