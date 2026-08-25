// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "VulkanCommon.h"

class VulkanInstance
{
public:
    VulkanInstance() = default;
    VulkanInstance(const VulkanInstance&) = delete;
    VulkanInstance& operator=(const VulkanInstance&) = delete;

    bool Create();
    void Destroy();

    VkInstance GetInstance() const { return instance; }

private:
    VkInstance instance = VK_NULL_HANDLE;

#if MX_GRAPHICS_VULKAN_DEBUG
    VkDebugUtilsMessengerEXT debugMsg = VK_NULL_HANDLE;
#endif

#if MX_GRAPHICS_VULKAN_DEBUG
    bool CreateDebugMsg(const VkDebugUtilsMessengerCreateInfoEXT& debugMsgInfo);
    void DestroyDebugMsg();
#endif
};
