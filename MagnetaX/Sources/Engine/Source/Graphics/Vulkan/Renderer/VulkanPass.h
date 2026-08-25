// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Graphics/Vulkan/VulkanCommon.h>

struct VulkanPassCreateInfo
{
    VulkanDevice* device = nullptr;
};

struct VulkanPassRenderInfo
{
    VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
};
