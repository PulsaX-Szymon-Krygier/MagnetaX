// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanImage.h"

#if MX_GRAPHICS_VULKAN
#include <Graphics/Vulkan/VulkanDevice.h>

VulkanImage::VulkanImage(VulkanImage&& other) noexcept
{
    device = other.device;

    image = other.image;
    imageView = other.imageView;
    format = other.format;
    memory = other.memory;

    ownsImage = other.ownsImage;

    other.device = VK_NULL_HANDLE;

    other.image = VK_NULL_HANDLE;
    other.imageView = VK_NULL_HANDLE;
    other.format = VK_FORMAT_UNDEFINED;
    other.memory = VK_NULL_HANDLE;

    other.ownsImage = false;
}

VulkanImage& VulkanImage::operator=(VulkanImage&& other) noexcept
{
    if (this == &other) return *this;

    Destroy();

    device = other.device;

    image = other.image;
    imageView = other.imageView;
    format = other.format;
    memory = other.memory;

    ownsImage = other.ownsImage;

    other.device = VK_NULL_HANDLE;

    other.image = VK_NULL_HANDLE;
    other.imageView = VK_NULL_HANDLE;
    other.format = VK_FORMAT_UNDEFINED;
    other.memory = VK_NULL_HANDLE;

    other.ownsImage = false;

    return *this;
}

bool VulkanImage::Create(VkDevice _device, VkImage _image, VkFormat _format, VkImageAspectFlags aspect)
{
    if (!_device || !_image || _format == VK_FORMAT_UNDEFINED) return false;

    Destroy();

    device = _device;
    image = _image;
    format = _format;
    ownsImage = false;

    if (!CreateImageView(aspect))
    {
        Destroy();
        return false;
    }

    return true;
}

bool VulkanImage::Create(VulkanDevice* _device, VkExtent2D extent, ImageFormat _format, VkImageUsageFlags usage,
    uint32 mipLevels, uint32 arrayLayers)
{
    if (!_device || extent.width == 0 || extent.height == 0 || mipLevels == 0 || arrayLayers == 0) return false;

    const VkDevice buffDevice = _device->GetDevice();
    if (!buffDevice) return false;

    const VkFormat buffFormat = VulkanImageFormat::FromImageFormat(_format);
    if (buffFormat == VK_FORMAT_UNDEFINED) return false;

    Destroy();

    device = buffDevice;
    format = buffFormat;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent = { extent.width, extent.height, 1 };
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = arrayLayers;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.pNext = nullptr;

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    ownsImage = true;

    VkMemoryRequirements memoryReqs{};
    vkGetImageMemoryRequirements(device, image, &memoryReqs);

    const uint32 memoryTypeIndex = _device->FindMemoryType(memoryReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (memoryTypeIndex == UINT32_MAX)
    {
        Destroy();
        return false;
    }

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryReqs.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    allocInfo.pNext = nullptr;

    if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    if (vkBindImageMemory(device, image, memory, 0) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    if (!CreateImageView(VulkanImageFormat::GetImageAspect(_format), mipLevels, arrayLayers))
    {
        Destroy();
        return false;
    }

    return true;
}

void VulkanImage::Destroy()
{
    if (device)
    {
        if (imageView) vkDestroyImageView(device, imageView, nullptr);

        if (ownsImage)
        {
            if (image) vkDestroyImage(device, image, nullptr);
            if (memory) vkFreeMemory(device, memory, nullptr);
        }
    }

    image = VK_NULL_HANDLE;
    imageView = VK_NULL_HANDLE;
    format = VK_FORMAT_UNDEFINED;
    memory = VK_NULL_HANDLE;

    device = VK_NULL_HANDLE;

    ownsImage = false;
}

bool VulkanImage::CreateImageView(VkImageAspectFlags aspect, uint32 mipLevels, uint32 arrayLayers)
{
    if (!device || !image || format == VK_FORMAT_UNDEFINED) return false;

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.subresourceRange.aspectMask = aspect;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = arrayLayers;
    viewInfo.pNext = nullptr;

    return vkCreateImageView(device, &viewInfo, nullptr, &imageView) == VK_SUCCESS;
}
#endif
