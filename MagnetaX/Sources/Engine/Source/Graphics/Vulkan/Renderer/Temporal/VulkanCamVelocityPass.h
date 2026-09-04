// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/Math/Matrix.h>
#include "../VulkanPass.h"
#include "../VulkanPipeline.h"

struct VulkanCamVelocityPassCreateInfo : VulkanPassCreateInfo
{
    const VulkanImage* depthImage = nullptr;
    VkFormat outFormat = VK_FORMAT_UNDEFINED;
};

struct VulkanCamVelocityPassRenderInfo : VulkanPassRenderInfo
{
    VkImageView targetView = VK_NULL_HANDLE;
    VkExtent2D extent{};

    Matrix4f invViewProj = Matrix4f::Identity();
    Matrix4f prevViewProj = Matrix4f::Identity();
};

class VulkanCamVelocityPass
{
public:
    VulkanCamVelocityPass() = default;

    bool Create(const VulkanCamVelocityPassCreateInfo& createInfo);
    void Destroy();

    void Record(const VulkanCamVelocityPassRenderInfo& renderInfo);

private:
    VkDevice device = VK_NULL_HANDLE;

    VulkanPipeline pipeline;

    VkSampler sampler = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
    VkDescriptorSet descSet = VK_NULL_HANDLE;
};
