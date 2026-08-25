// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "VulkanMinimal.h"

// Common macros
#ifndef MX_GRAPHICS_VULKAN_DEBUG
#define MX_GRAPHICS_VULKAN_DEBUG 0
#endif

#ifndef MX_GRAPHICS_VULKAN_MIN_VERSION
#define MX_GRAPHICS_VULKAN_MIN_VERSION VK_API_VERSION_1_3
#endif

// Common forward decls
class VulkanBuffer;
class VulkanDevice;
class VulkanImage;
class VulkanInstance;
class VulkanPipeline;
class VulkanTexture;
