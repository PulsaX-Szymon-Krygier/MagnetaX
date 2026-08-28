// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanEnvironment.h"

#if MX_GRAPHICS_VULKAN
#include <Graphics/Vulkan/VulkanInitializers.h>
#include <Graphics/Vulkan/VulkanDevice.h>
#include <Graphics/Vulkan/Resources/VulkanTexture.h>
#include <Graphics/Vulkan/Renderer/VulkanCommandPool.h>
#include <Graphics/Vulkan/Resources/VulkanImageFormat.h>
#include <utility>
#include <vector>
#include <span>
#include <array>

namespace
{
    constexpr uint32 CUBE_FACE_COUNT = 6;
    constexpr uint32 SPECULAR_ENV_SIZE = 256;
    constexpr uint32 SPECULAR_ENV_MIP_LEVELS = 5;
    constexpr uint32 BRDF_LUT_SIZE = 512;
    constexpr uint32 FALLBACK_SPECULAR_SIZE = 1;

    uint32 CalculateMipLevels(uint32 size)
    {
        uint32 mipLevels = 1;

        while (size > 1)
        {
            size >>= 1;
            ++mipLevels;
        }

        return mipLevels;
    }

    bool CreateLinearClampSampler(VkDevice device, float32 maxLod, VkSampler& sampler)
    {
        if (!device) return false;

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = maxLod;

        return vkCreateSampler(device, &samplerInfo, nullptr, &sampler) == VK_SUCCESS;
    }

    bool CreateSubresourceView(VkDevice device, VkImage image, VkFormat format, uint32 mipLevel, uint32 arrayLayer, VkImageView& view)
    {
        if (!device || !image || format == VK_FORMAT_UNDEFINED) return false;

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = mipLevel;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = arrayLayer;
        viewInfo.subresourceRange.layerCount = 1;

        return vkCreateImageView(device, &viewInfo, nullptr, &view) == VK_SUCCESS;
    }

    void DestroyImageViews(VkDevice device, std::span<VkImageView> views)
    {
        if (!device) return;

        for (VkImageView& view : views)
        {
            if (view) vkDestroyImageView(device, view, nullptr);
            view = VK_NULL_HANDLE;
        }
    }

    bool EndSubmitAndWait(VulkanDevice* device, VkCommandBuffer cmdBuffer)
    {
        if (!device || !cmdBuffer) return false;

        if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS) return false;

        VkCommandBufferSubmitInfo commandBufferInfo{};
        commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        commandBufferInfo.commandBuffer = cmdBuffer;

        VkSubmitInfo2 submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &commandBufferInfo;

        const VkQueue graphicsQueue = device->GetGraphicsQueue();
        if (!graphicsQueue) return false;

        if (vkQueueSubmit2(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) return false;

        return vkQueueWaitIdle(graphicsQueue) == VK_SUCCESS;
    }
}

bool VulkanEnvironment::Create(const VulkanEnvironmentCreateInfo& createInfo)
{
    if (!createInfo.device || !createInfo.device->GetDevice()) return false;

    Destroy();

    device = createInfo.device;

    const VkFormat environmentFormat = VulkanImageFormat::FromImageFormat(ImageFormat::RGBA16_FLOAT);

    if (environmentFormat == VK_FORMAT_UNDEFINED)
    {
        Destroy();
        return false;
    }

    VulkanEquirectPassCreateInfo equirectInfo{};
    equirectInfo.device = device;
    equirectInfo.outFormat = environmentFormat;

    if (!equirectPass.Create(equirectInfo))
    {
        Destroy();
        return false;
    }

    VulkanSpecularEnvPassCreateInfo specularInfo{};
    specularInfo.device = device;
    specularInfo.outFormat = environmentFormat;

    if (!specularEnvPass.Create(specularInfo))
    {
        Destroy();
        return false;
    }

    VulkanBRDFLUTPassCreateInfo brdfInfo{};
    brdfInfo.device = device;
    brdfInfo.outFormat = environmentFormat;

    if (!brdfLUTPass.Create(brdfInfo))
    {
        Destroy();
        return false;
    }

    if (!GenerateBRDFLUT())
    {
        Destroy();
        return false;
    }

    if (!CreateFallbackSpecular())
    {
        Destroy();
        return false;
    }

    return true;
}

void VulkanEnvironment::Destroy()
{
    DestroyEnvironmentResources();
    DestroyFallbackSpecular();
    DestroyBRDFLUT();

    brdfLUTPass.Destroy();
    specularEnvPass.Destroy();
    equirectPass.Destroy();

    device = nullptr;
}

bool VulkanEnvironment::SetSource(const VulkanEnvironmentSourceInfo& sourceInfo)
{
    if (!device || !device->GetDevice()) return false;
    if (!sourceInfo.pixels || sourceInfo.width == 0 || sourceInfo.height == 0) return false;
    if (sourceInfo.format != ImageFormat::RGBA32_FLOAT) return false;
    if (sourceInfo.width != sourceInfo.height * 2 || sourceInfo.width % 4 != 0) return false;

    const uint32 faceSize = sourceInfo.width / 4;
    if (faceSize == 0) return false;

    const VkDevice buffDevice = device->GetDevice();
    const uint32 environmentMipLevels = CalculateMipLevels(faceSize);

    VulkanTexture sourceTexture;

    VulkanImage newEnvironmentCubemap;
    VkSampler newEnvironmentSampler = VK_NULL_HANDLE;
    std::array<VkImageView, CUBE_FACE_COUNT> environmentFaceViews{};

    VulkanImage newSpecularCubemap;
    VkSampler newSpecularSampler = VK_NULL_HANDLE;
    std::vector<VkImageView> specularViews(SPECULAR_ENV_MIP_LEVELS * CUBE_FACE_COUNT, VK_NULL_HANDLE);

    auto cleanupNewResources = [&]()
        {
            DestroyImageViews(buffDevice, std::span<VkImageView>(specularViews));
            DestroyImageViews(buffDevice, std::span<VkImageView>(environmentFaceViews));

            if (newSpecularSampler) vkDestroySampler(buffDevice, newSpecularSampler, nullptr);
            if (newEnvironmentSampler) vkDestroySampler(buffDevice, newEnvironmentSampler, nullptr);

            newSpecularSampler = VK_NULL_HANDLE;
            newEnvironmentSampler = VK_NULL_HANDLE;

            newSpecularCubemap.Destroy();
            newEnvironmentCubemap.Destroy();
            sourceTexture.Destroy();
        };

    VulkanTextureCreateInfo textureInfo{};
    textureInfo.device = device;
    textureInfo.pixels = sourceInfo.pixels;
    textureInfo.width = sourceInfo.width;
    textureInfo.height = sourceInfo.height;
    textureInfo.format = sourceInfo.format;
    textureInfo.config.mipmaps = false;
    textureInfo.config.anisotropy = 1.0f;
    textureInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    textureInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    if (!sourceTexture.Create(textureInfo))
    {
        cleanupNewResources();
        return false;
    }

    VulkanImageCreateInfo environmentImageInfo{};
    environmentImageInfo.device = device;
    environmentImageInfo.extent = { faceSize, faceSize };
    environmentImageInfo.format = ImageFormat::RGBA16_FLOAT;
    environmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | 
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    environmentImageInfo.mipLevels = environmentMipLevels;
    environmentImageInfo.arrayLayers = CUBE_FACE_COUNT;
    environmentImageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    environmentImageInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;

    if (!newEnvironmentCubemap.Create(environmentImageInfo))
    {
        cleanupNewResources();
        return false;
    }

    for (uint32 faceIndex = 0; faceIndex < CUBE_FACE_COUNT; ++faceIndex)
    {
        if (!CreateSubresourceView(buffDevice, newEnvironmentCubemap.GetImage(),
            newEnvironmentCubemap.GetFormat(), 0, faceIndex, environmentFaceViews[faceIndex]))
        {
            cleanupNewResources();
            return false;
        }
    }

    if (!CreateLinearClampSampler(buffDevice, (float32)(environmentMipLevels - 1), newEnvironmentSampler))
    {
        cleanupNewResources();
        return false;
    }

    VulkanImageCreateInfo specularImageInfo{};
    specularImageInfo.device = device;
    specularImageInfo.extent = { SPECULAR_ENV_SIZE, SPECULAR_ENV_SIZE };
    specularImageInfo.format = ImageFormat::RGBA16_FLOAT;
    specularImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    specularImageInfo.mipLevels = SPECULAR_ENV_MIP_LEVELS;
    specularImageInfo.arrayLayers = CUBE_FACE_COUNT;
    specularImageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    specularImageInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;

    if (!newSpecularCubemap.Create(specularImageInfo))
    {
        cleanupNewResources();
        return false;
    }

    for (uint32 mipLevel = 0; mipLevel < SPECULAR_ENV_MIP_LEVELS; ++mipLevel)
    {
        for (uint32 faceIndex = 0; faceIndex < CUBE_FACE_COUNT; ++faceIndex)
        {
            const uint32 viewIndex = mipLevel * CUBE_FACE_COUNT + faceIndex;

            if (!CreateSubresourceView(buffDevice, newSpecularCubemap.GetImage(),
                newSpecularCubemap.GetFormat(), mipLevel, faceIndex, specularViews[viewIndex]))
            {
                cleanupNewResources();
                return false;
            }
        }
    }

    if (!CreateLinearClampSampler(buffDevice, (float32)(SPECULAR_ENV_MIP_LEVELS - 1), newSpecularSampler))
    {
        cleanupNewResources();
        return false;
    }

    VulkanCommandPool commandPool;

    if (!commandPool.Create(buffDevice, device->GetGraphicsQueueFamily()))
    {
        cleanupNewResources();
        return false;
    }

    const VkCommandBuffer cmdBuffer = commandPool.AllocateCommandBuffer();

    if (!cmdBuffer)
    {
        commandPool.Destroy();
        cleanupNewResources();
        return false;
    }

    const VkCommandBufferBeginInfo beginInfo = VulkanInitializers::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS)
    {
        commandPool.Destroy();
        cleanupNewResources();
        return false;
    }

    VulkanEquirectPassRenderInfo equirectRenderInfo{};
    equirectRenderInfo.cmdBuffer = cmdBuffer;
    equirectRenderInfo.sourceTexture = &sourceTexture;
    equirectRenderInfo.targetImage = newEnvironmentCubemap.GetImage();
    equirectRenderInfo.targetViews = std::span<const VkImageView>(environmentFaceViews);
    equirectRenderInfo.extent = { faceSize, faceSize };

    equirectPass.Record(equirectRenderInfo);

    int32 mipSize = (int32)faceSize;

    for (uint32 mipLevel = 1; mipLevel < environmentMipLevels; ++mipLevel)
    {
        VkImageMemoryBarrier2 mipBarriers[2]{};

        mipBarriers[0] = VulkanInitializers::ImageMemoryBarrier(newEnvironmentCubemap.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, mipLevel - 1, 1, 0, CUBE_FACE_COUNT);

        mipBarriers[1] = VulkanInitializers::ImageMemoryBarrier(newEnvironmentCubemap.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, mipLevel, 1, 0, CUBE_FACE_COUNT);

        VkDependencyInfo mipDependency{};
        mipDependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        mipDependency.imageMemoryBarrierCount = 2;
        mipDependency.pImageMemoryBarriers = mipBarriers;

        vkCmdPipelineBarrier2(cmdBuffer, &mipDependency);

        const int32 nextMipSize = mipSize > 1 ? mipSize / 2 : 1;

        VkImageBlit blit{};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = mipLevel - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = CUBE_FACE_COUNT;
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipSize, mipSize, 1 };

        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = mipLevel;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = CUBE_FACE_COUNT;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { nextMipSize, nextMipSize, 1 };

        vkCmdBlitImage(cmdBuffer, newEnvironmentCubemap.GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            newEnvironmentCubemap.GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        mipBarriers[0] = VulkanInitializers::ImageMemoryBarrier(newEnvironmentCubemap.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            VK_ACCESS_2_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
            mipLevel - 1, 1, 0, CUBE_FACE_COUNT);

        mipBarriers[1] = VulkanInitializers::ImageMemoryBarrier(newEnvironmentCubemap.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
            mipLevel, 1, 0, CUBE_FACE_COUNT);

        vkCmdPipelineBarrier2(cmdBuffer, &mipDependency);

        mipSize = nextMipSize;
    }

    VulkanSpecularEnvPassRenderInfo specularRenderInfo{};
    specularRenderInfo.cmdBuffer = cmdBuffer;
    specularRenderInfo.sourceView = newEnvironmentCubemap.GetImageView();
    specularRenderInfo.sourceSampler = newEnvironmentSampler;
    specularRenderInfo.targetImage = newSpecularCubemap.GetImage();
    specularRenderInfo.targetViews = std::span<const VkImageView>(specularViews);
    specularRenderInfo.extent = { SPECULAR_ENV_SIZE, SPECULAR_ENV_SIZE };
    specularRenderInfo.mipLevels = SPECULAR_ENV_MIP_LEVELS;

    specularEnvPass.Record(specularRenderInfo);

    if (!EndSubmitAndWait(device, cmdBuffer))
    {
        commandPool.Destroy();
        cleanupNewResources();
        return false;
    }

    commandPool.Destroy();

    DestroyImageViews(buffDevice, std::span<VkImageView>(specularViews));
    DestroyImageViews(buffDevice, std::span<VkImageView>(environmentFaceViews));
    sourceTexture.Destroy();

    DestroyEnvironmentResources();

    environmentCubemap = std::move(newEnvironmentCubemap);
    environmentSampler = newEnvironmentSampler;
    newEnvironmentSampler = VK_NULL_HANDLE;

    specularCubemap = std::move(newSpecularCubemap);
    specularSampler = newSpecularSampler;
    newSpecularSampler = VK_NULL_HANDLE;

    return true;
}

void VulkanEnvironment::ClearSource()
{
    if (!environmentCubemap.GetImage() && !specularCubemap.GetImage() && !environmentSampler && !specularSampler) return;

    if (device && device->GetDevice())
    {
        vkDeviceWaitIdle(device->GetDevice());
    }

    DestroyEnvironmentResources();
}

VulkanEnvironmentRenderData VulkanEnvironment::GetRenderData() const
{
    VulkanEnvironmentRenderData renderData{};

    if (environmentCubemap.GetImageView() && environmentSampler)
    {
        renderData.environmentView = environmentCubemap.GetImageView();
        renderData.environmentSampler = environmentSampler;
    }

    if (specularCubemap.GetImageView() && specularSampler)
    {
        renderData.specularView = specularCubemap.GetImageView();
        renderData.specularSampler = specularSampler;
    }
    else
    {
        renderData.specularView = fallbackSpecularCubemap.GetImageView();
        renderData.specularSampler = fallbackSpecularSampler;
    }

    renderData.brdfLUTView = brdfLUT.GetImageView();
    renderData.brdfLUTSampler = brdfLUTSampler;

    return renderData;
}

bool VulkanEnvironment::GenerateBRDFLUT()
{
    if (!device) return false;

    const VkDevice buffDevice = device->GetDevice();
    if (!buffDevice) return false;

    DestroyBRDFLUT();

    VulkanImageCreateInfo imageInfo{};
    imageInfo.device = device;
    imageInfo.extent = { BRDF_LUT_SIZE, BRDF_LUT_SIZE };
    imageInfo.format = ImageFormat::RGBA16_FLOAT;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    if (!brdfLUT.Create(imageInfo))
    {
        DestroyBRDFLUT();
        return false;
    }

    if (!CreateLinearClampSampler(buffDevice, 0.0f, brdfLUTSampler))
    {
        DestroyBRDFLUT();
        return false;
    }

    VulkanCommandPool commandPool;

    if (!commandPool.Create(buffDevice, device->GetGraphicsQueueFamily()))
    {
        DestroyBRDFLUT();
        return false;
    }

    const VkCommandBuffer cmdBuffer = commandPool.AllocateCommandBuffer();

    if (!cmdBuffer)
    {
        commandPool.Destroy();
        DestroyBRDFLUT();
        return false;
    }

    const VkCommandBufferBeginInfo beginInfo = VulkanInitializers::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS)
    {
        commandPool.Destroy();
        DestroyBRDFLUT();
        return false;
    }

    VulkanBRDFLUTPassRenderInfo renderInfo{};
    renderInfo.cmdBuffer = cmdBuffer;
    renderInfo.targetImage = brdfLUT.GetImage();
    renderInfo.targetView = brdfLUT.GetImageView();
    renderInfo.extent = { BRDF_LUT_SIZE, BRDF_LUT_SIZE };

    brdfLUTPass.Record(renderInfo);

    if (!EndSubmitAndWait(device, cmdBuffer))
    {
        commandPool.Destroy();
        DestroyBRDFLUT();
        return false;
    }

    commandPool.Destroy();

    return true;
}

bool VulkanEnvironment::CreateFallbackSpecular()
{
    if (!device) return false;

    const VkDevice buffDevice = device->GetDevice();
    if (!buffDevice) return false;

    DestroyFallbackSpecular();

    VulkanImageCreateInfo imageInfo{};
    imageInfo.device = device;
    imageInfo.extent = { FALLBACK_SPECULAR_SIZE, FALLBACK_SPECULAR_SIZE };
    imageInfo.format = ImageFormat::RGBA16_FLOAT;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.arrayLayers = CUBE_FACE_COUNT;
    imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    imageInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;

    if (!fallbackSpecularCubemap.Create(imageInfo))
    {
        DestroyFallbackSpecular();
        return false;
    }

    if (!CreateLinearClampSampler(buffDevice, 0.0f, fallbackSpecularSampler))
    {
        DestroyFallbackSpecular();
        return false;
    }

    VulkanCommandPool commandPool;

    if (!commandPool.Create(buffDevice, device->GetGraphicsQueueFamily()))
    {
        DestroyFallbackSpecular();
        return false;
    }

    const VkCommandBuffer cmdBuffer = commandPool.AllocateCommandBuffer();

    if (!cmdBuffer)
    {
        commandPool.Destroy();
        DestroyFallbackSpecular();
        return false;
    }

    const VkCommandBufferBeginInfo beginInfo = VulkanInitializers::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS)
    {
        commandPool.Destroy();
        DestroyFallbackSpecular();
        return false;
    }

    const VkImageMemoryBarrier2 toTransferBarrier = VulkanInitializers::ImageMemoryBarrier(fallbackSpecularCubemap.GetImage(),
        VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_NONE,
        VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, 0, 1, 0, CUBE_FACE_COUNT);

    VkDependencyInfo dependencyInfo{};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &toTransferBarrier;

    vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);

    VkClearColorValue clearColor{};
    clearColor.float32[3] = 1.0f;

    VkImageSubresourceRange clearRange{};
    clearRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clearRange.baseMipLevel = 0;
    clearRange.levelCount = 1;
    clearRange.baseArrayLayer = 0;
    clearRange.layerCount = CUBE_FACE_COUNT;

    vkCmdClearColorImage(cmdBuffer, fallbackSpecularCubemap.GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &clearRange);

    const VkImageMemoryBarrier2 toReadBarrier = VulkanInitializers::ImageMemoryBarrier(fallbackSpecularCubemap.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, 0, 1, 0, CUBE_FACE_COUNT);

    dependencyInfo.pImageMemoryBarriers = &toReadBarrier;

    vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);

    if (!EndSubmitAndWait(device, cmdBuffer))
    {
        commandPool.Destroy();
        DestroyFallbackSpecular();
        return false;
    }

    commandPool.Destroy();

    return true;
}

void VulkanEnvironment::DestroyEnvironmentResources()
{
    const VkDevice buffDevice = device ? device->GetDevice() : VK_NULL_HANDLE;

    if (buffDevice)
    {
        if (specularSampler) vkDestroySampler(buffDevice, specularSampler, nullptr);
        if (environmentSampler) vkDestroySampler(buffDevice, environmentSampler, nullptr);
    }

    specularSampler = VK_NULL_HANDLE;
    environmentSampler = VK_NULL_HANDLE;

    specularCubemap.Destroy();
    environmentCubemap.Destroy();
}

void VulkanEnvironment::DestroyBRDFLUT()
{
    const VkDevice buffDevice = device ? device->GetDevice() : VK_NULL_HANDLE;

    if (buffDevice && brdfLUTSampler) vkDestroySampler(buffDevice, brdfLUTSampler, nullptr);

    brdfLUTSampler = VK_NULL_HANDLE;
    brdfLUT.Destroy();
}

void VulkanEnvironment::DestroyFallbackSpecular()
{
    const VkDevice buffDevice = device ? device->GetDevice() : VK_NULL_HANDLE;

    if (buffDevice && fallbackSpecularSampler) vkDestroySampler(buffDevice, fallbackSpecularSampler, nullptr);

    fallbackSpecularSampler = VK_NULL_HANDLE;
    fallbackSpecularCubemap.Destroy();
}
#endif
