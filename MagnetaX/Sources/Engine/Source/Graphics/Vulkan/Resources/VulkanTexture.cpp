// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanTexture.h"

#if MX_GRAPHICS_VULKAN
#include "VulkanBuffer.h"
#include "../VulkanDevice.h"
#include "../VulkanInitializers.h"
#include <algorithm>
#include <cmath>

bool VulkanTexture::Create(VulkanDevice* _device, const uint8* pixels, uint32 width, uint32 height, ImageFormat format, const TextureConfig& config)
{
    if (!_device || !pixels || width == 0 || height == 0) return false;

    const VkDevice buffDevice = _device->GetDevice();
    if (!buffDevice) return false;

    const uint32 bytesPerPixel = ImageFormatUtils::GetBytesPerPixel(format);
    if (bytesPerPixel == 0) return false;

    Destroy();

    device = buffDevice;

    const VkDeviceSize dataSize = (VkDeviceSize)width * (VkDeviceSize)height * bytesPerPixel;

    VulkanBuffer stagingBuffer;
    const VkMemoryPropertyFlags stagingMemoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    if (!stagingBuffer.Create(_device, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingMemoryProperties))
    {
        Destroy();
        return false;
    }

    if (!stagingBuffer.Upload(pixels, dataSize))
    {
        stagingBuffer.Destroy();
        Destroy();
        return false;
    }

    const VkExtent2D extent{ width, height };
    const uint32 mipLevels = config.mipmaps ? (uint32)std::floor(std::log2((float32)std::max(width, height))) + 1 : 1;
    const VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    if (!image.Create(_device, extent, format, imageUsage, mipLevels))
    {
        stagingBuffer.Destroy();
        Destroy();
        return false;
    }

    if (!UploadPixels(_device, stagingBuffer.GetBuffer(), width, height, mipLevels))
    {
        stagingBuffer.Destroy();
        Destroy();
        return false;
    }

    stagingBuffer.Destroy();

    const GraphicsCapabilities& caps = _device->GetCapabilities();

    const bool anisoEnabled = caps.samplerAniso && config.anisotropy > 1.0f;
    const float32 aniso = anisoEnabled ? std::min(config.anisotropy, caps.maxSamplerAniso) : 1.0f;

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = (float32)(mipLevels - 1);
    samplerInfo.anisotropyEnable = anisoEnabled ? VK_TRUE : VK_FALSE;
    samplerInfo.maxAnisotropy = aniso;

    if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    return true;
}

void VulkanTexture::Destroy()
{
    if (device && sampler) vkDestroySampler(device, sampler, nullptr);

    sampler = VK_NULL_HANDLE;

    image.Destroy();

    device = VK_NULL_HANDLE;
}

bool VulkanTexture::UploadPixels(VulkanDevice* _device, VkBuffer stagingBuffer, uint32 width, uint32 height, uint32 mipLevels)
{
    if (!_device || !device || !stagingBuffer || width == 0 || height == 0 || mipLevels == 0) return false;

    VkCommandPool commandPool = VK_NULL_HANDLE;

    const VkCommandPoolCreateInfo poolInfo = VulkanInitializers::CommandPoolCreateInfo(
        _device->GetGraphicsQueueFamily(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
    );

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) return false;

    VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;

    const VkCommandBufferAllocateInfo allocInfo = VulkanInitializers::CommandBufferAllocateInfo(commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);

    if (vkAllocateCommandBuffers(device, &allocInfo, &cmdBuffer) != VK_SUCCESS)
    {
        vkDestroyCommandPool(device, commandPool, nullptr);
        return false;
    }

    const VkCommandBufferBeginInfo beginInfo = VulkanInitializers::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS)
    {
        vkDestroyCommandPool(device, commandPool, nullptr);
        return false;
    }

    const VkImageMemoryBarrier2 toTransferBarrier = VulkanInitializers::ImageMemoryBarrier(
        image.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, 0, mipLevels
    );

    VkDependencyInfo toTransferDependency{};
    toTransferDependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    toTransferDependency.imageMemoryBarrierCount = 1;
    toTransferDependency.pImageMemoryBarriers = &toTransferBarrier;

    vkCmdPipelineBarrier2(cmdBuffer, &toTransferDependency);

    VkBufferImageCopy copyRegion{};
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = 0;
    copyRegion.bufferImageHeight = 0;
    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.mipLevel = 0;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount = 1;
    copyRegion.imageOffset = { 0, 0, 0 };
    copyRegion.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(cmdBuffer, stagingBuffer, image.GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    int32 mipWidth = (int32)width;
    int32 mipHeight = (int32)height;

    for (uint32 mipLevel = 1; mipLevel < mipLevels; ++mipLevel)
    {
        const VkImageMemoryBarrier2 toTransferSourceBarrier = VulkanInitializers::ImageMemoryBarrier(
            image.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            VK_ACCESS_2_TRANSFER_READ_BIT, mipLevel - 1, 1
        );

        VkDependencyInfo toTransferSourceDependency{};
        toTransferSourceDependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        toTransferSourceDependency.imageMemoryBarrierCount = 1;
        toTransferSourceDependency.pImageMemoryBarriers = &toTransferSourceBarrier;

        vkCmdPipelineBarrier2(cmdBuffer, &toTransferSourceDependency);

        VkImageBlit blit{};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = mipLevel - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };

        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = mipLevel;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };

        vkCmdBlitImage(cmdBuffer, image.GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image.GetImage(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        const VkImageMemoryBarrier2 toShaderReadBarrier = VulkanInitializers::ImageMemoryBarrier(
            image.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, mipLevel - 1, 1
        );

        VkDependencyInfo toShaderReadDependency{};
        toShaderReadDependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        toShaderReadDependency.imageMemoryBarrierCount = 1;
        toShaderReadDependency.pImageMemoryBarriers = &toShaderReadBarrier;

        vkCmdPipelineBarrier2(cmdBuffer, &toShaderReadDependency);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    const VkImageMemoryBarrier2 lastMipToShaderReadBarrier = VulkanInitializers::ImageMemoryBarrier(
        image.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, mipLevels - 1, 1
    );

    VkDependencyInfo lastMipToShaderReadDependency{};
    lastMipToShaderReadDependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    lastMipToShaderReadDependency.imageMemoryBarrierCount = 1;
    lastMipToShaderReadDependency.pImageMemoryBarriers = &lastMipToShaderReadBarrier;

    vkCmdPipelineBarrier2(cmdBuffer, &lastMipToShaderReadDependency);

    if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS)
    {
        vkDestroyCommandPool(device, commandPool, nullptr);
        return false;
    }

    VkCommandBufferSubmitInfo cmdBufferInfo{};
    cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    cmdBufferInfo.commandBuffer = cmdBuffer;

    VkSubmitInfo2 submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &cmdBufferInfo;

    if (vkQueueSubmit2(_device->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        vkDestroyCommandPool(device, commandPool, nullptr);
        return false;
    }

    if (vkQueueWaitIdle(_device->GetGraphicsQueue()) != VK_SUCCESS)
    {
        vkDestroyCommandPool(device, commandPool, nullptr);
        return false;
    }

    vkDestroyCommandPool(device, commandPool, nullptr);

    return true;
}
#endif
