// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>

#if !MX_GRAPHICS_VULKAN
    #error Vulkan is not defined or/and not supported
#endif

#include <vulkan/vulkan.h>
