// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanEquirectPass.h"

#if MX_GRAPHICS_VULKAN
#include <MX/Generated/Shaders/Common/VulkanShaderFullscreenVert.h>
#include <MX/Generated/Shaders/Environment/VulkanShaderEquirectFrag.h>
#include <Graphics/Vulkan/VulkanDevice.h>
#include <Graphics/Vulkan/VulkanInitializers.h>
#include <Graphics/Vulkan/Resources/VulkanTexture.h>

bool VulkanEquirectPass::Create(const VulkanEquirectPassCreateInfo& createInfo)
{
    if (!createInfo.device || createInfo.outFormat == VK_FORMAT_UNDEFINED) return false;

    VkDevice buffDevice = createInfo.device->GetDevice();
    if (!buffDevice) return false;

    Destroy();

    device = buffDevice;

    VkDescriptorSetLayoutBinding binding{};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    const VkDescriptorSetLayoutCreateInfo layoutInfo = VulkanInitializers::DescriptorSetLayoutCreateInfo(1, &binding);

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descSetLayout) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    const VkDescriptorPoolCreateInfo poolInfo = VulkanInitializers::DescriptorPoolCreateInfo(1, 1, &poolSize);

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descPool) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    const VkDescriptorSetAllocateInfo allocInfo = VulkanInitializers::DescriptorSetAllocateInfo(descPool, 1, &descSetLayout);

    if (vkAllocateDescriptorSets(device, &allocInfo, &descSet) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    VkPushConstantRange pushConstRange{};
    pushConstRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstRange.offset = 0;
    pushConstRange.size = sizeof(uint32);

    VulkanPipelineCreateInfo pipelineInfo{};
    pipelineInfo.vertexShader = MX_GRAPHICS_VULKAN_SHADER_FULLSCREEN_VERT;
    pipelineInfo.vertexShaderSize = MX_GRAPHICS_VULKAN_SHADER_FULLSCREEN_VERT_SIZE;
    pipelineInfo.fragmentShader = MX_GRAPHICS_VULKAN_SHADER_EQUIRECT_FRAG;
    pipelineInfo.fragmentShaderSize = MX_GRAPHICS_VULKAN_SHADER_EQUIRECT_FRAG_SIZE;
    pipelineInfo.colorFormats = &createInfo.outFormat;
    pipelineInfo.colorFormatCount = 1;
    pipelineInfo.descriptorSetLayouts = &descSetLayout;
    pipelineInfo.descriptorSetLayoutCount = 1;
    pipelineInfo.pushConstantRanges = &pushConstRange;
    pipelineInfo.pushConstantRangeCount = 1;

    if (!pipeline.Create(device, pipelineInfo))
    {
        Destroy();
        return false;
    }

    return true;
}

void VulkanEquirectPass::Destroy()
{
    pipeline.Destroy();

    if (device)
    {
        if (descPool) vkDestroyDescriptorPool(device, descPool, nullptr);
        if (descSetLayout) vkDestroyDescriptorSetLayout(device, descSetLayout, nullptr);
    }

    descSet = VK_NULL_HANDLE;
    descPool = VK_NULL_HANDLE;
    descSetLayout = VK_NULL_HANDLE;

    device = VK_NULL_HANDLE;
}

void VulkanEquirectPass::Record(const VulkanEquirectPassRenderInfo& renderInfo)
{
    if (!device || !renderInfo.cmdBuffer || !renderInfo.sourceTexture || !renderInfo.targetImage) return;
    if (renderInfo.targetViews.size() != 6 || renderInfo.extent.width == 0 || renderInfo.extent.height == 0) return;
    if (!renderInfo.sourceTexture->GetImageView() || !renderInfo.sourceTexture->GetSampler()) return;

    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = renderInfo.sourceTexture->GetSampler();
    imageInfo.imageView = renderInfo.sourceTexture->GetImageView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet writeSet{};
    writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.dstSet = descSet;
    writeSet.dstBinding = 0;
    writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeSet.descriptorCount = 1;
    writeSet.pImageInfo = &imageInfo;
    writeSet.pNext = nullptr;

    vkUpdateDescriptorSets(device, 1, &writeSet, 0, nullptr);

    const VkImageMemoryBarrier2 toColorBarrier = VulkanInitializers::ImageMemoryBarrier(
        renderInfo.targetImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, 0, 1, 0, 6
    );

    VkDependencyInfo dependencyInfo{};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &toColorBarrier;
    dependencyInfo.pNext = nullptr;

    vkCmdPipelineBarrier2(renderInfo.cmdBuffer, &dependencyInfo);

    for (uint32 faceIndex = 0; faceIndex < renderInfo.targetViews.size(); ++faceIndex)
    {
        VkRenderingAttachmentInfo colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = renderInfo.targetViews[faceIndex];
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        colorAttachment.pNext = nullptr;

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea = { { 0, 0 }, renderInfo.extent };
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pNext = nullptr;

        vkCmdBeginRendering(renderInfo.cmdBuffer, &renderingInfo);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float32)renderInfo.extent.width;
        viewport.height = (float32)renderInfo.extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = renderInfo.extent;

        vkCmdSetViewport(renderInfo.cmdBuffer, 0, 1, &viewport);
        vkCmdSetScissor(renderInfo.cmdBuffer, 0, 1, &scissor);

        vkCmdBindPipeline(renderInfo.cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipeline());

        vkCmdBindDescriptorSets(renderInfo.cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipelineLayout(), 0, 1, &descSet, 0, nullptr);
        vkCmdPushConstants(renderInfo.cmdBuffer, pipeline.GetPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32), &faceIndex);

        vkCmdDraw(renderInfo.cmdBuffer, 3, 1, 0, 0);

        vkCmdEndRendering(renderInfo.cmdBuffer);
    }

    const VkImageMemoryBarrier2 toReadBarrier = VulkanInitializers::ImageMemoryBarrier(
        renderInfo.targetImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, 0, 1, 0, 6
    );

    dependencyInfo.pImageMemoryBarriers = &toReadBarrier;

    vkCmdPipelineBarrier2(renderInfo.cmdBuffer, &dependencyInfo);
}
#endif
