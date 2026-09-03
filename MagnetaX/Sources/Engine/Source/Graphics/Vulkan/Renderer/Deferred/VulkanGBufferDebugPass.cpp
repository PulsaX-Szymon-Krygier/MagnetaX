// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanGBufferDebugPass.h"

#if MX_GRAPHICS_VULKAN
#include <MX/Generated/Shaders/Common/VulkanShaderFullscreenVert.h>
#include <MX/Generated/Shaders/Deferred/VulkanShaderGBufferDebugFrag.h>
#include <Graphics/Vulkan/VulkanDevice.h>
#include "VulkanGBuffer.h"

namespace
{
    struct DebugPushConst
    {
        uint32 debugView = 0;
        uint32 velocityAvailable = 0;
    };
}

bool VulkanGBufferDebugPass::Create(const VulkanGBufferDebugPassCreateInfo& createInfo)
{
    if (!createInfo.device || !createInfo.gBuffer) return false;
    if (createInfo.outFormat == VK_FORMAT_UNDEFINED) return false;

    const VkDevice buffDevice = createInfo.device->GetDevice();
    if (!buffDevice) return false;

    Destroy();

    if (!gBufferBindings.Create(createInfo.device, createInfo.gBuffer))
    {
        Destroy();
        return false;
    }

    velocityAvailable = createInfo.gBuffer->GetVelocityImage().GetImageView() != VK_NULL_HANDLE;

    VkPushConstantRange pushConstRange{};
    pushConstRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstRange.offset = 0;
    pushConstRange.size = sizeof(DebugPushConst);

    const VkDescriptorSetLayout descSetLayout = gBufferBindings.GetDescriptorSetLayout();

    VulkanPipelineCreateInfo pipelineInfo{};
    pipelineInfo.vertexShader = MX_GRAPHICS_VULKAN_SHADER_FULLSCREEN_VERT;
    pipelineInfo.vertexShaderSize = MX_GRAPHICS_VULKAN_SHADER_FULLSCREEN_VERT_SIZE;
    pipelineInfo.fragmentShader = MX_GRAPHICS_VULKAN_SHADER_GBUFFERDEBUG_FRAG;
    pipelineInfo.fragmentShaderSize = MX_GRAPHICS_VULKAN_SHADER_GBUFFERDEBUG_FRAG_SIZE;
    pipelineInfo.colorFormats = &createInfo.outFormat;
    pipelineInfo.colorFormatCount = 1;
    pipelineInfo.descriptorSetLayouts = &descSetLayout;
    pipelineInfo.descriptorSetLayoutCount = 1;
    pipelineInfo.pushConstantRanges = &pushConstRange;
    pipelineInfo.pushConstantRangeCount = 1;

    if (!pipeline.Create(buffDevice, pipelineInfo))
    {
        Destroy();
        return false;
    }

    return true;
}

void VulkanGBufferDebugPass::Destroy()
{
    pipeline.Destroy();
    gBufferBindings.Destroy();

    velocityAvailable = false;
}

void VulkanGBufferDebugPass::Record(const VulkanGBufferDebugPassRenderInfo& renderInfo)
{
    if (!renderInfo.cmdBuffer || !renderInfo.targetView) return;
    if (renderInfo.extent.width == 0 || renderInfo.extent.height == 0) return;

    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = renderInfo.targetView;
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = { { 0, 0 }, renderInfo.extent };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

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

    const VkDescriptorSet descSet = gBufferBindings.GetDescriptorSet();

    vkCmdBindDescriptorSets(renderInfo.cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipelineLayout(),
        0, 1, &descSet, 0, nullptr);

    DebugPushConst debugPC{};
    debugPC.debugView = (uint32)renderInfo.debugView;
    debugPC.velocityAvailable = velocityAvailable ? 1u : 0u;

    vkCmdPushConstants(renderInfo.cmdBuffer, pipeline.GetPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DebugPushConst), &debugPC);

    vkCmdDraw(renderInfo.cmdBuffer, 3, 1, 0, 0);

    vkCmdEndRendering(renderInfo.cmdBuffer);
}
#endif
