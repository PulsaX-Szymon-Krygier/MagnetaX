// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanTAA.h"

#if MX_GRAPHICS_VULKAN
#include <Graphics/Vulkan/VulkanInitializers.h>
#include <bit>

namespace
{
    float32 Halton(uint32 index, uint32 base)
    {
        float32 result = 0.0f;
        float32 fraction = 1.0f;

        while (index > 0)
        {
            fraction /= (float32)base;
            result += fraction * (float32)(index % base);
            index /= base;
        }

        return result;
    }

    Vector2f CalculateJitter(uint64 frameIndex, VkExtent2D extent)
    {
        const uint32 sampleIndex = (uint32)((frameIndex - 1) % 16) + 1;

        const float32 jitterX = Halton(sampleIndex, 2) - 0.5f;
        const float32 jitterY = Halton(sampleIndex, 3) - 0.5f;

        return Vector2f(2.0f * jitterX / (float32)extent.width, 2.0f * jitterY / (float32)extent.height);
    }
}

bool VulkanTAA::Create(const VulkanTAACreateInfo& createInfo)
{
    if (!createInfo.device || !createInfo.currentColor || !createInfo.velocityImage || !createInfo.depthImage) return false;
    if (createInfo.extent.width == 0 || createInfo.extent.height == 0) return false;

    Destroy();

    extent = createInfo.extent;

    velocityImage = createInfo.velocityImage;
    depthImage = createInfo.depthImage;

    VulkanImageCreateInfo dilatedDepthInfo{};
    dilatedDepthInfo.device = createInfo.device;
    dilatedDepthInfo.extent = createInfo.extent;
    dilatedDepthInfo.format = ImageFormat::R32_FLOAT;
    dilatedDepthInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    if (!dilatedDepth.Create(dilatedDepthInfo))
    {
        Destroy();
        return false;
    }

    VulkanImageCreateInfo dilatedVelocityInfo{};
    dilatedVelocityInfo.device = createInfo.device;
    dilatedVelocityInfo.extent = createInfo.extent;
    dilatedVelocityInfo.format = ImageFormat::RGBA16_FLOAT;
    dilatedVelocityInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    if (!dilatedVelocity.Create(dilatedVelocityInfo))
    {
        Destroy();
        return false;
    }

    VulkanImageCreateInfo reconstructedDepthInfo{};
    reconstructedDepthInfo.device = createInfo.device;
    reconstructedDepthInfo.extent = createInfo.extent;
    reconstructedDepthInfo.format = ImageFormat::R32_UINT;
    reconstructedDepthInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    if (!reconstructedPrevDepth.Create(reconstructedDepthInfo))
    {
        Destroy();
        return false;
    }

    VulkanImageCreateInfo historyInfo{};
    historyInfo.device = createInfo.device;
    historyInfo.extent = createInfo.extent;
    historyInfo.format = ImageFormat::RGBA16_FLOAT;
    historyInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    for (VulkanImage& image : history)
    {
        if (!image.Create(historyInfo))
        {
            Destroy();
            return false;
        }
    }

    VulkanImageCreateInfo depthHistoryInfo{};
    depthHistoryInfo.device = createInfo.device;
    depthHistoryInfo.extent = createInfo.extent;
    depthHistoryInfo.format = ImageFormat::D32_FLOAT;
    depthHistoryInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    for (VulkanImage& image : depthHistory)
    {
        if (!image.Create(depthHistoryInfo))
        {
            Destroy();
            return false;
        }
    }

    for (VulkanImage& image : supportMean)
    {
        if (!image.Create(historyInfo))
        {
            Destroy();
            return false;
        }
    }

    for (VulkanImage& image : supportSigma)
    {
        if (!image.Create(historyInfo))
        {
            Destroy();
            return false;
        }
    }

    VulkanCamVelocityPassCreateInfo camVelocityInfo{};
    camVelocityInfo.device = createInfo.device;
    camVelocityInfo.depthImage = createInfo.depthImage;
    camVelocityInfo.outFormat = createInfo.velocityImage->GetFormat();

    if (!camVelocityPass.Create(camVelocityInfo))
    {
        Destroy();
        return false;
    }

    VulkanTAAPreparePassCreateInfo prepareInfo{};
    prepareInfo.device = createInfo.device;
    prepareInfo.depthImage = createInfo.depthImage;
    prepareInfo.velocityImage = createInfo.velocityImage;
    prepareInfo.dilatedDepthImage = &dilatedDepth;
    prepareInfo.dilatedVelocityImage = &dilatedVelocity;
    prepareInfo.reconstructedPrevDepthImage = &reconstructedPrevDepth;

    if (!preparePass.Create(prepareInfo))
    {
        Destroy();
        return false;
    }

    VulkanTAAPassCreateInfo taaInfo{};
    taaInfo.device = createInfo.device;
    taaInfo.currentColor = createInfo.currentColor;
    taaInfo.outFormat = history[0].GetFormat();
    taaInfo.velocityImage = createInfo.velocityImage;
    taaInfo.depthImage = createInfo.depthImage;
    taaInfo.reconstrPrevDepthImage = &reconstructedPrevDepth;

    if (!taaPass.Create(taaInfo))
    {
        Destroy();
        return false;
    }

    return true;
}

void VulkanTAA::Destroy()
{
    taaPass.Destroy();
    preparePass.Destroy();
    camVelocityPass.Destroy();

    reconstructedPrevDepth.Destroy();
    dilatedVelocity.Destroy();
    dilatedDepth.Destroy();

    for (VulkanImage& image : supportSigma)
    {
        image.Destroy();
    }

    for (VulkanImage& image : supportMean)
    {
        image.Destroy();
    }

    for (VulkanImage& image : depthHistory)
    {
        image.Destroy();
    }

    for (VulkanImage& image : history)
    {
        image.Destroy();
    }

    historyLayouts = { VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED };
    depthHistoryLayouts = { VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED };

    reconstructedPrevDepthLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    dilatedVelocityLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    dilatedDepthLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    supportMeanLayouts = { VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED };
    supportSigmaLayouts = { VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED };

    ResetHistory();

    velocityImage = nullptr;
    depthImage = nullptr;

    extent = {};
}

void VulkanTAA::RecordCameraVelocity(VkCommandBuffer cmdBuffer, const Matrix4f& invViewProj, const Matrix4f& prevViewProj)
{
    if (!cmdBuffer || !velocityImage) return;
    if (extent.width == 0 || extent.height == 0) return;

    const VkImageMemoryBarrier2 velocityWriteBarrier = VulkanInitializers::ImageMemoryBarrier(velocityImage->GetImage(), 
        VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, 
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

    VkDependencyInfo dependencyInfo{};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &velocityWriteBarrier;

    vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);

    VulkanCamVelocityPassRenderInfo camVelocityInfo{};
    camVelocityInfo.cmdBuffer = cmdBuffer;
    camVelocityInfo.targetView = velocityImage->GetImageView();
    camVelocityInfo.extent = extent;
    camVelocityInfo.invViewProj = invViewProj;
    camVelocityInfo.prevViewProj = prevViewProj;

    camVelocityPass.Record(camVelocityInfo);

    const VkImageMemoryBarrier2 velocityReadBarrier = VulkanInitializers::ImageMemoryBarrier(velocityImage->GetImage(), 
        VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, 
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

    dependencyInfo.pImageMemoryBarriers = &velocityReadBarrier;

    vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);
}

VkImageView VulkanTAA::Resolve(const VulkanTAAResolveInfo& resolveInfo)
{
    if (!resolveInfo.cmdBuffer || !depthImage) return VK_NULL_HANDLE;
    if (extent.width == 0 || extent.height == 0) return VK_NULL_HANDLE;

    RecordPrepare(resolveInfo.cmdBuffer, resolveInfo.jitterUV);

    const uint32 historyWriteIndex = 1u - historyReadIndex;

    VulkanImage& historyRead = history[historyReadIndex];
    VulkanImage& historyWrite = history[historyWriteIndex];

    VulkanImage& supportMeanRead = supportMean[historyReadIndex];
    VulkanImage& supportSigmaRead = supportSigma[historyReadIndex];

    VulkanImage& supportMeanWrite = supportMean[historyWriteIndex];
    VulkanImage& supportSigmaWrite = supportSigma[historyWriteIndex];

    VulkanImage& depthHistoryRead = depthHistory[historyReadIndex];
    VulkanImage& depthHistoryWrite = depthHistory[historyWriteIndex];

    VkImageLayout& historyReadLayout = historyLayouts[historyReadIndex];
    VkImageLayout& historyWriteLayout = historyLayouts[historyWriteIndex];
    VkImageLayout& depthHistoryWriteLayout = depthHistoryLayouts[historyWriteIndex];

    VkImageLayout& supportMeanWriteLayout = supportMeanLayouts[historyWriteIndex];
    VkImageLayout& supportSigmaWriteLayout = supportSigmaLayouts[historyWriteIndex];

    VkImageLayout& supportMeanReadLayout = supportMeanLayouts[historyReadIndex];
    VkImageLayout& supportSigmaReadLayout = supportSigmaLayouts[historyReadIndex];

    VkDependencyInfo dependencyInfo{};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;

    VkPipelineStageFlags2 depthHistoryWriteSrcStage = VK_PIPELINE_STAGE_2_NONE;
    VkAccessFlags2 depthHistoryWriteSrcAccess = VK_ACCESS_2_NONE;

    if (depthHistoryWriteLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        depthHistoryWriteSrcStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        depthHistoryWriteSrcAccess = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    }

    const VkImageMemoryBarrier2 depthCopyBarriers[2] =
    {
        VulkanInitializers::ImageMemoryBarrier(depthImage->GetImage(), VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, 
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT),

        VulkanInitializers::ImageMemoryBarrier(depthHistoryWrite.GetImage(), VK_IMAGE_ASPECT_DEPTH_BIT, depthHistoryWriteLayout, 
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, depthHistoryWriteSrcStage, depthHistoryWriteSrcAccess, VK_PIPELINE_STAGE_2_TRANSFER_BIT, 
            VK_ACCESS_2_TRANSFER_WRITE_BIT)
    };

    dependencyInfo.imageMemoryBarrierCount = 2;
    dependencyInfo.pImageMemoryBarriers = depthCopyBarriers;

    vkCmdPipelineBarrier2(resolveInfo.cmdBuffer, &dependencyInfo);

    VkImageCopy depthCopy{};
    depthCopy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthCopy.srcSubresource.mipLevel = 0;
    depthCopy.srcSubresource.baseArrayLayer = 0;
    depthCopy.srcSubresource.layerCount = 1;
    depthCopy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthCopy.dstSubresource.mipLevel = 0;
    depthCopy.dstSubresource.baseArrayLayer = 0;
    depthCopy.dstSubresource.layerCount = 1;
    depthCopy.extent = { extent.width, extent.height, 1 };

    vkCmdCopyImage(resolveInfo.cmdBuffer, depthImage->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, depthHistoryWrite.GetImage(), 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &depthCopy);

    const VkImageMemoryBarrier2 depthReadBarriers[2] =
    {
        VulkanInitializers::ImageMemoryBarrier(depthImage->GetImage(), VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, 
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT),

        VulkanInitializers::ImageMemoryBarrier(depthHistoryWrite.GetImage(), VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, 
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT)
    };

    dependencyInfo.imageMemoryBarrierCount = 2;
    dependencyInfo.pImageMemoryBarriers = depthReadBarriers;

    vkCmdPipelineBarrier2(resolveInfo.cmdBuffer, &dependencyInfo);

    depthHistoryWriteLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    if (historyReadLayout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        const VkImageMemoryBarrier2 historyReadBarrier = VulkanInitializers::ImageMemoryBarrier(historyRead.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, 
            historyReadLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE, 
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

        dependencyInfo.imageMemoryBarrierCount = 1;
        dependencyInfo.pImageMemoryBarriers = &historyReadBarrier;

        vkCmdPipelineBarrier2(resolveInfo.cmdBuffer, &dependencyInfo);

        historyReadLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    if (supportMeanReadLayout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        const VkImageMemoryBarrier2 supportMeanReadBarrier = VulkanInitializers::ImageMemoryBarrier(supportMeanRead.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, 
            supportMeanReadLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, 
            VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

        dependencyInfo.imageMemoryBarrierCount = 1;
        dependencyInfo.pImageMemoryBarriers = &supportMeanReadBarrier;

        vkCmdPipelineBarrier2(resolveInfo.cmdBuffer, &dependencyInfo);

        supportMeanReadLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    if (supportSigmaReadLayout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        const VkImageMemoryBarrier2 supportSigmaReadBarrier = VulkanInitializers::ImageMemoryBarrier(supportSigmaRead.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, 
            supportSigmaReadLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, 
            VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

        dependencyInfo.imageMemoryBarrierCount = 1;
        dependencyInfo.pImageMemoryBarriers = &supportSigmaReadBarrier;

        vkCmdPipelineBarrier2(resolveInfo.cmdBuffer, &dependencyInfo);

        supportSigmaReadLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    VkPipelineStageFlags2 historyWriteSrcStage = VK_PIPELINE_STAGE_2_NONE;
    VkAccessFlags2 historyWriteSrcAccess = VK_ACCESS_2_NONE;

    if (historyWriteLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        historyWriteSrcStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        historyWriteSrcAccess = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    }

    VkPipelineStageFlags2 supportMeanWriteSrcStage = VK_PIPELINE_STAGE_2_NONE;
    VkAccessFlags2 supportMeanWriteSrcAccess = VK_ACCESS_2_NONE;

    if (supportMeanWriteLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        supportMeanWriteSrcStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        supportMeanWriteSrcAccess = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    }

    VkPipelineStageFlags2 supportSigmaWriteSrcStage = VK_PIPELINE_STAGE_2_NONE;
    VkAccessFlags2 supportSigmaWriteSrcAccess = VK_ACCESS_2_NONE;

    if (supportSigmaWriteLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        supportSigmaWriteSrcStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        supportSigmaWriteSrcAccess = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    }

    const VkImageMemoryBarrier2 historyWriteBarriers[3] =
    {
        VulkanInitializers::ImageMemoryBarrier(historyWrite.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, historyWriteLayout, 
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, historyWriteSrcStage, historyWriteSrcAccess, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT),

        VulkanInitializers::ImageMemoryBarrier(supportMeanWrite.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, supportMeanWriteLayout, 
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, supportMeanWriteSrcStage, supportMeanWriteSrcAccess, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT),

        VulkanInitializers::ImageMemoryBarrier(supportSigmaWrite.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, supportSigmaWriteLayout, 
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, supportSigmaWriteSrcStage, supportSigmaWriteSrcAccess, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT)
    };

    dependencyInfo.imageMemoryBarrierCount = 3;
    dependencyInfo.pImageMemoryBarriers = historyWriteBarriers;

    vkCmdPipelineBarrier2(resolveInfo.cmdBuffer, &dependencyInfo);

    historyWriteLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    supportMeanWriteLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    supportSigmaWriteLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VulkanTAAPassRenderInfo taaInfo{};
    taaInfo.cmdBuffer = resolveInfo.cmdBuffer;
    taaInfo.historyView = historyRead.GetImageView();
    taaInfo.targetView = historyWrite.GetImageView();
    taaInfo.previousDepthView = historyValid ? depthHistoryRead.GetImageView() : depthHistoryWrite.GetImageView();
    taaInfo.extent = extent;
    taaInfo.jitterUV = resolveInfo.jitterUV;
    taaInfo.prevJitterUV = prevJitterUV;
    taaInfo.feedbackMin = resolveInfo.feedbackMin;
    taaInfo.feedbackMax = resolveInfo.feedbackMax;
    taaInfo.historyValid = historyValid;
    taaInfo.targetSupportMeanView = supportMeanWrite.GetImageView();
    taaInfo.targetSupportSigmaView = supportSigmaWrite.GetImageView();
    taaInfo.previousSupportMeanView = supportMeanRead.GetImageView();
    taaInfo.previousSupportSigmaView = supportSigmaRead.GetImageView();
    taaInfo.currentViewProj = resolveInfo.currentViewProj;
    taaInfo.previousInvViewProj = resolveInfo.previousInvViewProj;
    taaInfo.nearPlane = resolveInfo.nearPlane;
    taaInfo.farPlane = resolveInfo.farPlane;
    taaInfo.projScale = resolveInfo.projScale;

    taaPass.Record(taaInfo);

    const VkImageMemoryBarrier2 outputBarriers[3] =
    {
        VulkanInitializers::ImageMemoryBarrier(historyWrite.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, 
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT),

        VulkanInitializers::ImageMemoryBarrier(supportMeanWrite.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, 
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT),

        VulkanInitializers::ImageMemoryBarrier(supportSigmaWrite.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, 
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT)
    };

    dependencyInfo.imageMemoryBarrierCount = 3;
    dependencyInfo.pImageMemoryBarriers = outputBarriers;

    vkCmdPipelineBarrier2(resolveInfo.cmdBuffer, &dependencyInfo);

    historyWriteLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    supportMeanWriteLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    supportSigmaWriteLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    historyReadIndex = historyWriteIndex;
    frameIndex++;
    historyValid = true;
    prevJitterUV = resolveInfo.jitterUV;

    return historyWrite.GetImageView();
}

void VulkanTAA::ResetHistory()
{
    historyReadIndex = 0;
    frameIndex = 0;
    historyValid = false;
    prevJitterUV = {};
}

Vector2f VulkanTAA::GetProjectionJitter() const
{
    if (!historyValid || frameIndex == 0) return Vector2f(0.0f);
    if (extent.width == 0 || extent.height == 0) return Vector2f(0.0f);

    return CalculateJitter(frameIndex, extent);
}

void VulkanTAA::RecordPrepare(VkCommandBuffer cmdBuffer, const Vector2f& jitterUV)
{
    if (!cmdBuffer) return;

    VkDependencyInfo dependencyInfo{};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;

    VkPipelineStageFlags2 reconstructedSrcStage = VK_PIPELINE_STAGE_2_NONE;
    VkAccessFlags2 reconstructedSrcAccess = VK_ACCESS_2_NONE;

    if (reconstructedPrevDepthLayout == VK_IMAGE_LAYOUT_GENERAL)
    {
        reconstructedSrcStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        reconstructedSrcAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
    }

    const VkImageMemoryBarrier2 reconstructedClearBarrier = VulkanInitializers::ImageMemoryBarrier(reconstructedPrevDepth.GetImage(), 
        VK_IMAGE_ASPECT_COLOR_BIT, reconstructedPrevDepthLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, reconstructedSrcStage, 
        reconstructedSrcAccess, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT);

    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &reconstructedClearBarrier;

    vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);

    reconstructedPrevDepthLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    VkClearColorValue clearValue{};
    clearValue.uint32[0] = std::bit_cast<uint32>(1.0f);

    VkImageSubresourceRange clearRange{};
    clearRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clearRange.baseMipLevel = 0;
    clearRange.levelCount = 1;
    clearRange.baseArrayLayer = 0;
    clearRange.layerCount = 1;

    vkCmdClearColorImage(cmdBuffer, reconstructedPrevDepth.GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &clearRange);

    const VkImageMemoryBarrier2 reconstructedWriteBarrier = VulkanInitializers::ImageMemoryBarrier(reconstructedPrevDepth.GetImage(), 
        VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, 
        VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);

    dependencyInfo.pImageMemoryBarriers = &reconstructedWriteBarrier;

    vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);

    reconstructedPrevDepthLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkPipelineStageFlags2 dilatedDepthSrcStage = VK_PIPELINE_STAGE_2_NONE;
    VkAccessFlags2 dilatedDepthSrcAccess = VK_ACCESS_2_NONE;

    if (dilatedDepthLayout == VK_IMAGE_LAYOUT_GENERAL)
    {
        dilatedDepthSrcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        dilatedDepthSrcAccess = VK_ACCESS_2_SHADER_WRITE_BIT;
    }

    VkPipelineStageFlags2 dilatedVelocitySrcStage = VK_PIPELINE_STAGE_2_NONE;
    VkAccessFlags2 dilatedVelocitySrcAccess = VK_ACCESS_2_NONE;

    if (dilatedVelocityLayout == VK_IMAGE_LAYOUT_GENERAL)
    {
        dilatedVelocitySrcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        dilatedVelocitySrcAccess = VK_ACCESS_2_SHADER_WRITE_BIT;
    }

    const VkImageMemoryBarrier2 writeBarriers[2] =
    {
        VulkanInitializers::ImageMemoryBarrier(dilatedDepth.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, dilatedDepthLayout, VK_IMAGE_LAYOUT_GENERAL, 
            dilatedDepthSrcStage, dilatedDepthSrcAccess, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT),

        VulkanInitializers::ImageMemoryBarrier(dilatedVelocity.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, dilatedVelocityLayout, VK_IMAGE_LAYOUT_GENERAL, 
            dilatedVelocitySrcStage, dilatedVelocitySrcAccess, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT)
    };

    dependencyInfo.imageMemoryBarrierCount = 2;
    dependencyInfo.pImageMemoryBarriers = writeBarriers;

    vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);

    dilatedDepthLayout = VK_IMAGE_LAYOUT_GENERAL;
    dilatedVelocityLayout = VK_IMAGE_LAYOUT_GENERAL;

    VulkanTAAPreparePassRenderInfo prepareInfo{};
    prepareInfo.cmdBuffer = cmdBuffer;
    prepareInfo.extent = extent;
    prepareInfo.jitterUV = jitterUV;
    prepareInfo.prevJitterUV = prevJitterUV;

    preparePass.Record(prepareInfo);

    const VkImageMemoryBarrier2 reconstructedReadBarrier = VulkanInitializers::ImageMemoryBarrier(reconstructedPrevDepth.GetImage(), 
        VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, 
        VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT);

    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &reconstructedReadBarrier;

    vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);
}
#endif
