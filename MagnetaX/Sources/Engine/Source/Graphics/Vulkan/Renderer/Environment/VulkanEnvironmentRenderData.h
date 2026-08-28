// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Graphics/Vulkan/VulkanCommon.h>

struct VulkanEnvironmentRenderData
{
    VkImageView environmentView = VK_NULL_HANDLE;
    VkSampler environmentSampler = VK_NULL_HANDLE;

    VkImageView specularView = VK_NULL_HANDLE;
    VkSampler specularSampler = VK_NULL_HANDLE;

    VkImageView brdfLUTView = VK_NULL_HANDLE;
    VkSampler brdfLUTSampler = VK_NULL_HANDLE;
};
