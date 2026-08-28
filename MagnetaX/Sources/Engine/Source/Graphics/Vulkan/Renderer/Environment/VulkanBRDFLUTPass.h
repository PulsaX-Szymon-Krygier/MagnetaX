// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "../VulkanPass.h"
#include "../VulkanPipeline.h"

struct VulkanBRDFLUTPassCreateInfo : VulkanPassCreateInfo
{
    VkFormat outFormat = VK_FORMAT_UNDEFINED;
};

struct VulkanBRDFLUTPassRenderInfo : VulkanPassRenderInfo
{
    VkImage targetImage = VK_NULL_HANDLE;
    VkImageView targetView = VK_NULL_HANDLE;
    VkExtent2D extent{};
};

class VulkanBRDFLUTPass
{
public:
    VulkanBRDFLUTPass() = default;

    bool Create(const VulkanBRDFLUTPassCreateInfo& createInfo);
    void Destroy();

    void Record(const VulkanBRDFLUTPassRenderInfo& renderInfo);

private:
    VkDevice device = VK_NULL_HANDLE;

    VulkanPipeline pipeline;
};
