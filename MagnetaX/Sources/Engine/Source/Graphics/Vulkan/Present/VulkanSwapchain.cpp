// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanSwapchain.h"

#if MX_GRAPHICS_VULKAN
#include <Graphics/Vulkan/VulkanDevice.h>
#include <algorithm>
#include <vector>
#include <utility>

namespace
{
    VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
    {
        if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) return { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

        for (const VkSurfaceFormatKHR& format : formats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return format;
        }

        return formats[0];
    }

    VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& presentModes)
    {
        for (VkPresentModeKHR presentMode : presentModes)
        {
            if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) return presentMode;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& surfaceCaps, VkExtent2D requestedExtent)
    {
        if (surfaceCaps.currentExtent.width != UINT32_MAX) return surfaceCaps.currentExtent;

        VkExtent2D extent{};
        extent.width = std::clamp(requestedExtent.width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
        extent.height = std::clamp(requestedExtent.height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);

        return extent;
    }

    VkCompositeAlphaFlagBitsKHR ChooseCompositeAlpha(const VkSurfaceCapabilitiesKHR& surfaceCaps)
    {
        const VkCompositeAlphaFlagBitsKHR modes[] =
        {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
        };

        for (VkCompositeAlphaFlagBitsKHR mode : modes)
        {
            if (surfaceCaps.supportedCompositeAlpha & mode) return mode;
        }

        return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }
}

bool VulkanSwapchain::Create(const VulkanSwapchainCreateInfo& createInfo)
{
    if (!createInfo.device || !createInfo.surface) return false;
    if (createInfo.extent.width == 0 || createInfo.extent.height == 0) return false;

    const VkDevice buffDevice = createInfo.device->GetDevice();
    const VkPhysicalDevice physicalDevice = createInfo.device->GetPhysicalDevice();

    if (!buffDevice || !physicalDevice) return false;

    Destroy();

    VkSurfaceCapabilitiesKHR surfaceCaps{};

    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, createInfo.surface, &surfaceCaps) != VK_SUCCESS)
    {
        return false;
    }

    uint32 formatCount = 0;

    if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, createInfo.surface, &formatCount, nullptr) != VK_SUCCESS)
    {
        return false;
    }

    if (formatCount == 0) return false;

    std::vector<VkSurfaceFormatKHR> formats(formatCount);

    if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, createInfo.surface, &formatCount, formats.data()) != VK_SUCCESS)
    {
        return false;
    }

    uint32 presentModeCount = 0;

    if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, createInfo.surface, &presentModeCount, nullptr) != VK_SUCCESS)
    {
        return false;
    }

    if (presentModeCount == 0) return false;

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);

    if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, createInfo.surface,
        &presentModeCount, presentModes.data()) != VK_SUCCESS)
    {
        return false;
    }

    const VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(formats);
    const VkPresentModeKHR presentMode = ChoosePresentMode(presentModes);
    const VkExtent2D swapchainExtent = ChooseExtent(surfaceCaps, createInfo.extent);

    if (swapchainExtent.width == 0 || swapchainExtent.height == 0) return false;

    uint32 imageCount = surfaceCaps.minImageCount + 1;

    if (surfaceCaps.maxImageCount > 0 && imageCount > surfaceCaps.maxImageCount)
    {
        imageCount = surfaceCaps.maxImageCount;
    }

    const uint32 graphicsQueueFamily = createInfo.device->GetGraphicsQueueFamily();
    const uint32 presentQueueFamily = createInfo.device->GetPresentQueueFamily();

    if (graphicsQueueFamily == UINT32_MAX || presentQueueFamily == UINT32_MAX) return false;

    const uint32 queueFamilyIndices[] =
    {
        graphicsQueueFamily,
        presentQueueFamily
    };

    VkSwapchainCreateInfoKHR swapchainInfo{};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = createInfo.surface;
    swapchainInfo.minImageCount = imageCount;
    swapchainInfo.imageFormat = surfaceFormat.format;
    swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainInfo.imageExtent = swapchainExtent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (graphicsQueueFamily == presentQueueFamily)
    {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.queueFamilyIndexCount = 0;
        swapchainInfo.pQueueFamilyIndices = nullptr;
    }
    else
    {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainInfo.queueFamilyIndexCount = 2;
        swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
    }

    swapchainInfo.preTransform = surfaceCaps.currentTransform;
    swapchainInfo.compositeAlpha = ChooseCompositeAlpha(surfaceCaps);
    swapchainInfo.presentMode = presentMode;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
    swapchainInfo.pNext = nullptr;

    VkSwapchainKHR buffSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(buffDevice, &swapchainInfo, nullptr, &buffSwapchain) != VK_SUCCESS) return false;

    uint32 swapchainImageCount = 0;

    if (vkGetSwapchainImagesKHR(buffDevice, buffSwapchain, &swapchainImageCount, nullptr) != VK_SUCCESS || swapchainImageCount == 0)
    {
        vkDestroySwapchainKHR(buffDevice, buffSwapchain, nullptr);
        return false;
    }

    std::vector<VkImage> swapchainImages(swapchainImageCount);

    if (vkGetSwapchainImagesKHR(buffDevice, buffSwapchain, &swapchainImageCount, swapchainImages.data()) != VK_SUCCESS)
    {
        vkDestroySwapchainKHR(buffDevice, buffSwapchain, nullptr);
        return false;
    }

    device = buffDevice;
    swapchain = buffSwapchain;
    format = surfaceFormat.format;
    extent = swapchainExtent;

    images.reserve(swapchainImageCount);

    for (VkImage image : swapchainImages)
    {
        VulkanImage buffImage;

        if (!buffImage.Create(device, image, format, VK_IMAGE_ASPECT_COLOR_BIT))
        {
            Destroy();
            return false;
        }

        images.push_back(std::move(buffImage));
    }

    return true;
}

void VulkanSwapchain::Destroy()
{
    for (VulkanImage& image : images) image.Destroy();
    images.clear();

    if (device && swapchain) vkDestroySwapchainKHR(device, swapchain, nullptr);

    swapchain = VK_NULL_HANDLE;
    format = VK_FORMAT_UNDEFINED;
    extent = {};

    device = VK_NULL_HANDLE;
}
#endif
