// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/Math/Matrix.h>
#include <Graphics/Vulkan/Resources/VulkanImage.h>
#include "../VulkanDrawItem.h"
#include "../VulkanPass.h"
#include "../VulkanPipeline.h"
#include <span>
#include <vector>

struct VulkanShadowDepthPassCreateInfo : VulkanPassCreateInfo
{
    VkExtent2D extent{ 2048, 2048 };
    uint32 layerCount = 1;
};

struct VulkanShadowDepthPassRenderInfo : VulkanPassRenderInfo
{
    std::span<const VulkanDrawItem> drawItems;
    std::span<const Matrix4f> lightViewProjs;
};

class VulkanShadowDepthPass
{
public:
    bool Create(const VulkanShadowDepthPassCreateInfo& createInfo);
    void Destroy();

    void Record(const VulkanShadowDepthPassRenderInfo& renderInfo);

    VkImageView GetShadowMapView() const { return shadowMapLayerViews.empty() ? VK_NULL_HANDLE : shadowMapLayerViews[0]; }
    VkImageView GetShadowMapArrayView() const { return shadowMap.GetImageView(); }
    VkFormat GetShadowMapFormat() const { return shadowMap.GetFormat(); }
    VkSampler GetSampler() const { return sampler; }

    VkExtent2D GetExtent() const { return extent; }
    uint32 GetLayerCount() const { return layerCount; }

private:
    VkDevice device = VK_NULL_HANDLE;

    VulkanImage shadowMap;
    VulkanPipeline pipeline;

    VkSampler sampler = VK_NULL_HANDLE;
    VkExtent2D extent{};
    uint32 layerCount = 0;

    VkImageLayout shadowMapLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    std::vector<VkImageView> shadowMapLayerViews;
};
