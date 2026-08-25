// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Graphics/GraphicsSystemFactory.h>
#include <MX/Graphics/AbstractGraphicsSystem.h>

#if MX_GRAPHICS_VULKAN
    #include "Vulkan/VulkanGraphicsSystem.h"
#endif

std::unique_ptr<AbstractGraphicsSystem> CreateGraphicsSystem()
{
    #if MX_GRAPHICS_VULKAN
    return std::make_unique<VulkanGraphicsSystem>();
    #else
    return nullptr;
    #endif
}
