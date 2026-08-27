// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanRenderer.h"

#if MX_GRAPHICS_VULKAN
#include <MX/Graphics/Renderer/UI/UIRenderData.h>
#include <Graphics/Renderer/Scene/RenderSceneData.h>
#include <Graphics/Renderer/Shadow/ShadowFrameBuilder.h>
#include <Graphics/Renderer/Shadow/ShadowFrameData.h>
#include "../Present/VulkanPresentContext.h"
#include "../VulkanDevice.h"
#include "../VulkanInitializers.h"
#include <array>

bool VulkanRenderer::Create(const VulkanRendererCreateInfo& createInfo)
{
    if (!createInfo.device || !createInfo.presentContext || !createInfo.materialDescSetLayout) return false;

    const VkDevice buffDevice = createInfo.device->GetDevice();
    if (!buffDevice) return false;

    VulkanSwapchain& swapchain = createInfo.presentContext->GetSwapchain();

    const VkExtent2D extent = swapchain.GetExtent();
    const VkFormat swapchainFormat = swapchain.GetFormat();

    if (extent.width == 0 || extent.height == 0) return false;
    if (swapchainFormat == VK_FORMAT_UNDEFINED || swapchain.GetImages().empty()) return false;
    if (createInfo.config.shadows.directional.resolution == 0 || createInfo.config.shadows.spot.resolution == 0) return false;

    Destroy();

    device = createInfo.device;
    presentContext = createInfo.presentContext;
    config = createInfo.config;

    if (!commandPool.Create(buffDevice, device->GetGraphicsQueueFamily()))
    {
        Destroy();
        return false;
    }

    cmdBuffer = commandPool.AllocateCommandBuffer();

    if (!cmdBuffer)
    {
        Destroy();
        return false;
    }

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = nullptr;

    if (vkCreateSemaphore(buffDevice, &semaphoreInfo, nullptr, &imageAvailable) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    renderFinishedSemaphores.resize(swapchain.GetImages().size());

    for (VkSemaphore& semaphore : renderFinishedSemaphores)
    {
        if (vkCreateSemaphore(buffDevice, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
        {
            Destroy();
            return false;
        }
    }

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    fenceInfo.pNext = nullptr;

    if (vkCreateFence(buffDevice, &fenceInfo, nullptr, &inFlight) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    VulkanShadowDepthPassCreateInfo directionalShadowInfo{};
    directionalShadowInfo.device = device;
    directionalShadowInfo.extent = { config.shadows.directional.resolution, config.shadows.directional.resolution };
    directionalShadowInfo.layerCount = MX_GRAPHICS_DIRECTIONAL_SHADOW_CASCADE_COUNT;

    if (!directionalShadowPass.Create(directionalShadowInfo))
    {
        Destroy();
        return false;
    }

    VulkanShadowDepthPassCreateInfo spotShadowInfo{};
    spotShadowInfo.device = device;
    spotShadowInfo.extent = { config.shadows.spot.resolution, config.shadows.spot.resolution };
    spotShadowInfo.layerCount = 1;

    if (!spotShadowPass.Create(spotShadowInfo))
    {
        Destroy();
        return false;
    }

    VulkanGBufferPassCreateInfo gBufferInfo{};
    gBufferInfo.device = device;
    gBufferInfo.extent = extent;
    gBufferInfo.materialDescSetLayout = createInfo.materialDescSetLayout;

    if (!gBufferPass.Create(gBufferInfo))
    {
        Destroy();
        return false;
    }

    const VkImageUsageFlags sceneColorUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    VulkanImageCreateInfo sceneColorImgInfo{};
    sceneColorImgInfo.device = device;
    sceneColorImgInfo.extent = extent;
    sceneColorImgInfo.format = ImageFormat::RGBA16_FLOAT;
    sceneColorImgInfo.usage = sceneColorUsage;

    if (!sceneColor.Create(sceneColorImgInfo))
    {
        Destroy();
        return false;
    }

    VulkanLightingPassCreateInfo lightingInfo{};
    lightingInfo.device = device;
    lightingInfo.gBuffer = &gBufferPass.GetGBuffer();
    lightingInfo.outFormat = sceneColor.GetFormat();
    lightingInfo.directionalShadowView = directionalShadowPass.GetShadowMapArrayView();
    lightingInfo.directionalShadowSampler = directionalShadowPass.GetSampler();
    lightingInfo.spotShadowView = spotShadowPass.GetShadowMapView();
    lightingInfo.spotShadowSampler = spotShadowPass.GetSampler();

    if (!lightingPass.Create(lightingInfo))
    {
        Destroy();
        return false;
    }

    VulkanGBufferDebugPassCreateInfo gBufferDebugInfo{};
    gBufferDebugInfo.device = device;
    gBufferDebugInfo.gBuffer = &gBufferPass.GetGBuffer();
    gBufferDebugInfo.outFormat = swapchainFormat;

    if (!gBufferDebugPass.Create(gBufferDebugInfo))
    {
        Destroy();
        return false;
    }

    VulkanPostFXPassCreateInfo postFXInfo{};
    postFXInfo.device = device;
    postFXInfo.srcImage = &sceneColor;
    postFXInfo.outFormat = swapchainFormat;

    if (!postFXPass.Create(postFXInfo))
    {
        Destroy();
        return false;
    }

    VulkanUIPassCreateInfo uiInfo{};
    uiInfo.device = device;
    uiInfo.outFormat = swapchainFormat;

    if (!uiPass.Create(uiInfo))
    {
        Destroy();
        return false;
    }

    swapchainImageLayouts.assign(swapchain.GetImages().size(), VK_IMAGE_LAYOUT_UNDEFINED);

    return true;
}

void VulkanRenderer::Destroy()
{
    if (device && device->GetDevice()) vkDeviceWaitIdle(device->GetDevice());

    uiPass.Destroy();
    postFXPass.Destroy();

    gBufferDebugPass.Destroy();
    lightingPass.Destroy();

    sceneColor.Destroy();
    sceneColorLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    gBufferPass.Destroy();

    spotShadowPass.Destroy();
    directionalShadowPass.Destroy();

    if (device)
    {
        const VkDevice buffDevice = device->GetDevice();

        if (buffDevice)
        {
            if (inFlight) vkDestroyFence(buffDevice, inFlight, nullptr);

            for (VkSemaphore semaphore : renderFinishedSemaphores)
            {
                if (semaphore) vkDestroySemaphore(buffDevice, semaphore, nullptr);
            }

            if (imageAvailable) vkDestroySemaphore(buffDevice, imageAvailable, nullptr);
        }
    }

    inFlight = VK_NULL_HANDLE;

    renderFinishedSemaphores.clear();

    imageAvailable = VK_NULL_HANDLE;

    cmdBuffer = VK_NULL_HANDLE;
    commandPool.Destroy();

    swapchainImageLayouts.clear();

    config = {};
    presentContext = nullptr;
    device = nullptr;
}

VulkanFrameResult VulkanRenderer::DrawFrame(std::span<const VulkanDrawItem> drawItems, const RenderSceneData& sceneData, const UIRenderData& uiData)
{
    if (!device || !presentContext || !cmdBuffer) return VulkanFrameResult::FAILED;
    if (!imageAvailable || !inFlight) return VulkanFrameResult::FAILED;

    const VkDevice buffDevice = device->GetDevice();
    if (!buffDevice) return VulkanFrameResult::FAILED;

    VulkanSwapchain& swapchain = presentContext->GetSwapchain();

    if (swapchain.GetImages().empty()) return VulkanFrameResult::FAILED;
    if (swapchainImageLayouts.size() != swapchain.GetImages().size()) return VulkanFrameResult::FAILED;
    if (renderFinishedSemaphores.size() != swapchain.GetImages().size()) return VulkanFrameResult::FAILED;

    const VkExtent2D extent = swapchain.GetExtent();

    if (extent.width == 0 || extent.height == 0) return VulkanFrameResult::FAILED;

    if (vkWaitForFences(buffDevice, 1, &inFlight, VK_TRUE, UINT64_MAX) != VK_SUCCESS) return VulkanFrameResult::FAILED;

    uint32 imageIndex = 0;

    const VkResult acquireResult = vkAcquireNextImageKHR(
        buffDevice, swapchain.GetSwapchain(), UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &imageIndex
    );

    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) return VulkanFrameResult::RECREATE;
    if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR) return VulkanFrameResult::FAILED;
    if (imageIndex >= swapchain.GetImages().size()) return VulkanFrameResult::FAILED;

    const bool suboptimal = acquireResult == VK_SUBOPTIMAL_KHR;

    const VulkanImage& targetImage = swapchain.GetImages()[imageIndex];
    const VkSemaphore renderFinished = renderFinishedSemaphores[imageIndex];

    if (vkResetCommandBuffer(cmdBuffer, 0) != VK_SUCCESS) return VulkanFrameResult::FAILED;

    const VkCommandBufferBeginInfo beginInfo = VulkanInitializers::CommandBufferBeginInfo();

    if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS) return VulkanFrameResult::FAILED;

    const ShadowFrameData shadowData = BuildShadowFrameData(sceneData, config.shadows);

    VulkanShadowDepthPassRenderInfo directionalShadowInfo{};
    directionalShadowInfo.cmdBuffer = cmdBuffer;
    directionalShadowInfo.drawItems = shadowData.directional.lightIndex != MX_GRAPHICS_INVALID_SHADOW_LIGHT_INDEX ?
        drawItems : std::span<const VulkanDrawItem>{};
    directionalShadowInfo.lightViewProjs = shadowData.directional.viewProjs;

    directionalShadowPass.Record(directionalShadowInfo);

    const std::array<Matrix4f, 1> spotViewProjs{ shadowData.spot.viewProj };

    VulkanShadowDepthPassRenderInfo spotShadowInfo{};
    spotShadowInfo.cmdBuffer = cmdBuffer;
    spotShadowInfo.drawItems = shadowData.spot.lightIndex != MX_GRAPHICS_INVALID_SHADOW_LIGHT_INDEX ?
        drawItems : std::span<const VulkanDrawItem>{};
    spotShadowInfo.lightViewProjs = spotViewProjs;

    spotShadowPass.Record(spotShadowInfo);

    VulkanGBufferPassRenderInfo gBufferInfo{};
    gBufferInfo.cmdBuffer = cmdBuffer;
    gBufferInfo.drawItems = drawItems;

    gBufferPass.Record(gBufferInfo);

    VkImageLayout& targetLayout = swapchainImageLayouts[imageIndex];

    const VkImageMemoryBarrier2 targetWriteBarrier = VulkanInitializers::ImageMemoryBarrier(
        targetImage.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, targetLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
    );

    VkDependencyInfo dependencyInfo{};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &targetWriteBarrier;
    dependencyInfo.pNext = nullptr;

    vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);

    targetLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    if (debugView == GraphicsDebugView::FINAL)
    {
        VkPipelineStageFlags2 sceneColorSrcStage = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2 sceneColorSrcAccess = VK_ACCESS_2_NONE;

        if (sceneColorLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            sceneColorSrcStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
            sceneColorSrcAccess = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
        }

        const VkImageMemoryBarrier2 sceneColorWriteBarrier = VulkanInitializers::ImageMemoryBarrier(
            sceneColor.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, sceneColorLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            sceneColorSrcStage, sceneColorSrcAccess, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
        );

        dependencyInfo.pImageMemoryBarriers = &sceneColorWriteBarrier;

        vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);

        sceneColorLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VulkanLightingPassRenderInfo lightingInfo{};
        lightingInfo.cmdBuffer = cmdBuffer;
        lightingInfo.targetView = sceneColor.GetImageView();
        lightingInfo.extent = extent;
        lightingInfo.sceneData = &sceneData;
        lightingInfo.shadowData = &shadowData;

        lightingPass.Record(lightingInfo);

        const VkImageMemoryBarrier2 sceneColorReadBarrier = VulkanInitializers::ImageMemoryBarrier(
            sceneColor.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            VK_ACCESS_2_SHADER_SAMPLED_READ_BIT
        );

        dependencyInfo.pImageMemoryBarriers = &sceneColorReadBarrier;

        vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);

        sceneColorLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VulkanPostFXPassRenderInfo postFXInfo{};
        postFXInfo.cmdBuffer = cmdBuffer;
        postFXInfo.targetView = targetImage.GetImageView();
        postFXInfo.extent = extent;

        postFXPass.Record(postFXInfo);
    }
    else
    {
        GBufferDebugView gBufferView = GBufferDebugView::ALBEDO;

        switch (debugView)
        {
        case GraphicsDebugView::NORMAL:
            gBufferView = GBufferDebugView::NORMAL;
            break;

        case GraphicsDebugView::MATERIAL:
            gBufferView = GBufferDebugView::MATERIAL;
            break;

        case GraphicsDebugView::DEPTH:
            gBufferView = GBufferDebugView::DEPTH;
            break;

        default:
            break;
        }

        VulkanGBufferDebugPassRenderInfo gBufferDebugInfo{};
        gBufferDebugInfo.cmdBuffer = cmdBuffer;
        gBufferDebugInfo.targetView = targetImage.GetImageView();
        gBufferDebugInfo.extent = extent;
        gBufferDebugInfo.debugView = gBufferView;

        gBufferDebugPass.Record(gBufferDebugInfo);
    }

    const VkImageMemoryBarrier2 targetUIBarrier = VulkanInitializers::ImageMemoryBarrier(
        targetImage.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
    );

    dependencyInfo.pImageMemoryBarriers = &targetUIBarrier;

    vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);

    VulkanUIPassRenderInfo uiInfo{};
    uiInfo.cmdBuffer = cmdBuffer;
    uiInfo.targetView = targetImage.GetImageView();
    uiInfo.extent = extent;
    uiInfo.uiData = &uiData;

    uiPass.Record(uiInfo);

    const VkImageMemoryBarrier2 toPresentBarrier = VulkanInitializers::ImageMemoryBarrier(
        targetImage.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE
    );

    dependencyInfo.pImageMemoryBarriers = &toPresentBarrier;

    vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);

    targetLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS) return VulkanFrameResult::FAILED;

    VkSemaphoreSubmitInfo waitSemaphoreInfo{};
    waitSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    waitSemaphoreInfo.semaphore = imageAvailable;
    waitSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    waitSemaphoreInfo.pNext = nullptr;

    VkCommandBufferSubmitInfo cmdBufferInfo{};
    cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    cmdBufferInfo.commandBuffer = cmdBuffer;
    cmdBufferInfo.pNext = nullptr;

    VkSemaphoreSubmitInfo signalSemaphoreInfo{};
    signalSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    signalSemaphoreInfo.semaphore = renderFinished;
    signalSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    signalSemaphoreInfo.pNext = nullptr;

    VkSubmitInfo2 submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.waitSemaphoreInfoCount = 1;
    submitInfo.pWaitSemaphoreInfos = &waitSemaphoreInfo;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &cmdBufferInfo;
    submitInfo.signalSemaphoreInfoCount = 1;
    submitInfo.pSignalSemaphoreInfos = &signalSemaphoreInfo;
    submitInfo.pNext = nullptr;

    if (vkResetFences(buffDevice, 1, &inFlight) != VK_SUCCESS) return VulkanFrameResult::FAILED;

    if (vkQueueSubmit2(device->GetGraphicsQueue(), 1, &submitInfo, inFlight) != VK_SUCCESS) return VulkanFrameResult::FAILED;

    const VkSwapchainKHR buffSwapchain = swapchain.GetSwapchain();

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinished;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &buffSwapchain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pNext = nullptr;

    const VkResult presentResult = vkQueuePresentKHR(device->GetPresentQueue(), &presentInfo);

    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) return VulkanFrameResult::RECREATE;
    if (presentResult != VK_SUCCESS) return VulkanFrameResult::FAILED;

    return suboptimal ? VulkanFrameResult::RECREATE : VulkanFrameResult::SUCCESS;
}
#endif
