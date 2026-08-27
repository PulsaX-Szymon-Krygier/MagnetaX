// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanShadowDepthPass.h"

#if MX_GRAPHICS_VULKAN
#include <MX/Generated/Shaders/Shadow/VulkanShaderShadowDepthVert.h>
#include <MX/Graphics/Resources/MeshVertex.h>
#include <Graphics/Vulkan/Resources/VulkanMesh.h>
#include <Graphics/Vulkan/VulkanDevice.h>
#include <Graphics/Vulkan/VulkanInitializers.h>
#include <cstddef>

bool VulkanShadowDepthPass::Create(const VulkanShadowDepthPassCreateInfo& createInfo)
{
    if (!createInfo.device || createInfo.layerCount == 0) return false;
    if (createInfo.extent.width == 0 || createInfo.extent.height == 0) return false;

    const VkDevice buffDevice = createInfo.device->GetDevice();
    if (!buffDevice) return false;

    Destroy();

    device = buffDevice;
    extent = createInfo.extent;
    layerCount = createInfo.layerCount;

    const VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    VulkanImageCreateInfo imageInfo{};
    imageInfo.device = createInfo.device;
    imageInfo.extent = extent;
    imageInfo.format = ImageFormat::D32_FLOAT;
    imageInfo.usage = imageUsage;
    imageInfo.arrayLayers = layerCount;
    imageInfo.viewType = layerCount > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;

    if (!shadowMap.Create(imageInfo))
    {
        Destroy();
        return false;
    }

    shadowMapLayerViews.resize(layerCount);

    for (uint32 i = 0; i < layerCount; ++i)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = shadowMap.GetImage();
        viewInfo.format = shadowMap.GetFormat();
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = i;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &viewInfo, nullptr, &shadowMapLayerViews[i]) != VK_SUCCESS)
        {
            Destroy();
            return false;
        }
    }

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.compareEnable = VK_TRUE;
    samplerInfo.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    VkVertexInputBindingDescription vertexBinding{};
    vertexBinding.binding = 0;
    vertexBinding.stride = sizeof(MeshVertex);
    vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription positionAttribute{};
    positionAttribute.location = 0;
    positionAttribute.binding = 0;
    positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionAttribute.offset = offsetof(MeshVertex, position);

    VkPushConstantRange pushConstRange{};
    pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstRange.offset = 0;
    pushConstRange.size = sizeof(Matrix4f);

    VulkanPipelineCreateInfo pipelineInfo{};
    pipelineInfo.vertexShader = MX_GRAPHICS_VULKAN_SHADER_SHADOWDEPTH_VERT;
    pipelineInfo.vertexShaderSize = MX_GRAPHICS_VULKAN_SHADER_SHADOWDEPTH_VERT_SIZE;
    pipelineInfo.vertexBindings = &vertexBinding;
    pipelineInfo.vertexBindingCount = 1;
    pipelineInfo.vertexAttributes = &positionAttribute;
    pipelineInfo.vertexAttributeCount = 1;
    pipelineInfo.depthFormat = shadowMap.GetFormat();
    pipelineInfo.depthTest = true;
    pipelineInfo.depthWrite = true;
    pipelineInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    pipelineInfo.depthBias = true;
    pipelineInfo.pushConstantRanges = &pushConstRange;
    pipelineInfo.pushConstantRangeCount = 1;
    pipelineInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    pipelineInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    if (!pipeline.Create(device, pipelineInfo))
    {
        Destroy();
        return false;
    }

    return true;
}

void VulkanShadowDepthPass::Destroy()
{
    pipeline.Destroy();

    if (device)
    {
        if (sampler) vkDestroySampler(device, sampler, nullptr);

        for (VkImageView imageView : shadowMapLayerViews)
        {
            if (imageView) vkDestroyImageView(device, imageView, nullptr);
        }
    }

    sampler = VK_NULL_HANDLE;
    shadowMapLayerViews.clear();

    shadowMap.Destroy();

    extent = {};
    layerCount = 0;
    shadowMapLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    device = VK_NULL_HANDLE;
}

void VulkanShadowDepthPass::Record(const VulkanShadowDepthPassRenderInfo& renderInfo)
{
    if (!renderInfo.cmdBuffer) return;
    if (renderInfo.lightViewProjs.size() != layerCount) return;
    if (shadowMapLayerViews.size() != layerCount) return;

    const bool firstUse = shadowMapLayout == VK_IMAGE_LAYOUT_UNDEFINED;
    const VkPipelineStageFlags2 srcStage = firstUse ? VK_PIPELINE_STAGE_2_NONE : VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    const VkAccessFlags2 srcAccess = firstUse ? VK_ACCESS_2_NONE : VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;

    const VkImageMemoryBarrier2 toDepthBarrier = VulkanInitializers::ImageMemoryBarrier(
        shadowMap.GetImage(), VK_IMAGE_ASPECT_DEPTH_BIT, shadowMapLayout, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        srcStage, srcAccess, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, 0, 1, 0, layerCount
    );

    VkDependencyInfo dependencyInfo{};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &toDepthBarrier;

    vkCmdPipelineBarrier2(renderInfo.cmdBuffer, &dependencyInfo);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float32)extent.width;
    viewport.height = (float32)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = extent;

    vkCmdSetViewport(renderInfo.cmdBuffer, 0, 1, &viewport);
    vkCmdSetScissor(renderInfo.cmdBuffer, 0, 1, &scissor);
    vkCmdBindPipeline(renderInfo.cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipeline());
    vkCmdSetDepthBias(renderInfo.cmdBuffer, 0.0f, 0.0f, 2.0f);

    for (uint32 layer = 0; layer < layerCount; ++layer)
    {
        VkRenderingAttachmentInfo depthAttachment{};
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageView = shadowMapLayerViews[layer];
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.clearValue.depthStencil = { 1.0f, 0 };

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea = { { 0, 0 }, extent };
        renderingInfo.layerCount = 1;
        renderingInfo.pDepthAttachment = &depthAttachment;

        vkCmdBeginRendering(renderInfo.cmdBuffer, &renderingInfo);

        for (const VulkanDrawItem& drawItem : renderInfo.drawItems)
        {
            if (!drawItem.mesh) continue;

            const VkBuffer vertexBuffer = drawItem.mesh->GetVertexBuffer();
            const VkDeviceSize vertexOffset = 0;

            vkCmdBindVertexBuffers(renderInfo.cmdBuffer, 0, 1, &vertexBuffer, &vertexOffset);
            vkCmdBindIndexBuffer(renderInfo.cmdBuffer, drawItem.mesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

            const Matrix4f shadowMVP = (renderInfo.lightViewProjs[layer] * drawItem.model).Transposed();

            vkCmdPushConstants(renderInfo.cmdBuffer, pipeline.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Matrix4f), &shadowMVP);
            vkCmdDrawIndexed(renderInfo.cmdBuffer, drawItem.mesh->GetIndexCount(), 1, 0, 0, 0);
        }

        vkCmdEndRendering(renderInfo.cmdBuffer);
    }

    const VkImageMemoryBarrier2 toShaderReadBarrier = VulkanInitializers::ImageMemoryBarrier(
        shadowMap.GetImage(), VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, 0, 1, 0, layerCount
    );

    dependencyInfo.pImageMemoryBarriers = &toShaderReadBarrier;
    vkCmdPipelineBarrier2(renderInfo.cmdBuffer, &dependencyInfo);

    shadowMapLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
}
#endif
