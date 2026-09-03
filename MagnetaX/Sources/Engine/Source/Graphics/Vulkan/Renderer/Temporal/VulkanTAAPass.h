// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/Math/Matrix.h>
#include <MX/Core/Math/Vector.h>
#include "../VulkanPass.h"
#include "../VulkanPipeline.h"

struct VulkanTAAPassCreateInfo : VulkanPassCreateInfo
{
    VulkanImage* currentColor = nullptr;
    VkFormat outFormat = VK_FORMAT_UNDEFINED;
    const VulkanImage* depthImage = nullptr;
};

struct VulkanTAAPassRenderInfo : VulkanPassRenderInfo
{
    VkImageView historyView = VK_NULL_HANDLE;
    VkImageView targetView = VK_NULL_HANDLE;
    VkExtent2D extent{};
    float32 historyWeight = 0.875f;
    bool historyValid = false;
    Matrix4f reprojection = Matrix4f::Identity();
    Vector2f projectionJitter{};
};

class VulkanTAAPass
{
public:
    VulkanTAAPass() = default;

    bool Create(const VulkanTAAPassCreateInfo& createInfo);
    void Destroy();

    void Record(const VulkanTAAPassRenderInfo& renderInfo);

private:
    VkDevice device = VK_NULL_HANDLE;

    VulkanPipeline pipeline;

    VkSampler sampler = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
    VkDescriptorSet descSet = VK_NULL_HANDLE;
};
