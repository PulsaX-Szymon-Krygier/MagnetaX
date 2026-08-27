// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanUIPass.h"

#if MX_GRAPHICS_VULKAN
#include <MX/Generated/Shaders/UI/VulkanShaderUIVert.h>
#include <MX/Generated/Shaders/UI/VulkanShaderUIFrag.h>
#include <MX/UI/UIFont.h>
#include <Graphics/Vulkan/VulkanDevice.h>
#include <Graphics/Vulkan/VulkanInitializers.h>
#include <cstddef>

namespace
{
    struct UIPushConst
    {
        Vector2f viewportSize;
    };
}

bool VulkanUIPass::Create(const VulkanUIPassCreateInfo& createInfo)
{
    if (!createInfo.device || createInfo.outFormat == VK_FORMAT_UNDEFINED) return false;

    const VkDevice buffDevice = createInfo.device->GetDevice();
    if (!buffDevice) return false;

    Destroy();

    device = createInfo.device;

    VkDescriptorSetLayoutBinding fontBinding{};
    fontBinding.binding = 0;
    fontBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    fontBinding.descriptorCount = 1;
    fontBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    const VkDescriptorSetLayoutCreateInfo layoutInfo = VulkanInitializers::DescriptorSetLayoutCreateInfo(1, &fontBinding);

    if (vkCreateDescriptorSetLayout(buffDevice, &layoutInfo, nullptr, &descSetLayout) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    const VkDescriptorPoolCreateInfo poolInfo = VulkanInitializers::DescriptorPoolCreateInfo(1, 1, &poolSize);

    if (vkCreateDescriptorPool(buffDevice, &poolInfo, nullptr, &descPool) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    const VkDescriptorSetAllocateInfo allocInfo = VulkanInitializers::DescriptorSetAllocateInfo(descPool, 1, &descSetLayout);

    if (vkAllocateDescriptorSets(buffDevice, &allocInfo, &descSet) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    VkVertexInputBindingDescription vertexBinding{};
    vertexBinding.binding = 0;
    vertexBinding.stride = sizeof(UIVertex);
    vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vertexAttribs[3]{};

    vertexAttribs[0].location = 0;
    vertexAttribs[0].binding = 0;
    vertexAttribs[0].format = VK_FORMAT_R32G32_SFLOAT;
    vertexAttribs[0].offset = offsetof(UIVertex, position);

    vertexAttribs[1].location = 1;
    vertexAttribs[1].binding = 0;
    vertexAttribs[1].format = VK_FORMAT_R32G32_SFLOAT;
    vertexAttribs[1].offset = offsetof(UIVertex, uv);

    vertexAttribs[2].location = 2;
    vertexAttribs[2].binding = 0;
    vertexAttribs[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vertexAttribs[2].offset = offsetof(UIVertex, color);

    VkPushConstantRange pushConstRange{};
    pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstRange.offset = 0;
    pushConstRange.size = sizeof(UIPushConst);

    VulkanPipelineCreateInfo pipelineInfo{};
    pipelineInfo.vertexShader = MX_GRAPHICS_VULKAN_SHADER_UI_VERT;
    pipelineInfo.vertexShaderSize = MX_GRAPHICS_VULKAN_SHADER_UI_VERT_SIZE;
    pipelineInfo.fragmentShader = MX_GRAPHICS_VULKAN_SHADER_UI_FRAG;
    pipelineInfo.fragmentShaderSize = MX_GRAPHICS_VULKAN_SHADER_UI_FRAG_SIZE;
    pipelineInfo.vertexBindings = &vertexBinding;
    pipelineInfo.vertexBindingCount = 1;
    pipelineInfo.vertexAttributes = vertexAttribs;
    pipelineInfo.vertexAttributeCount = 3;
    pipelineInfo.colorFormats = &createInfo.outFormat;
    pipelineInfo.colorFormatCount = 1;
    pipelineInfo.descriptorSetLayouts = &descSetLayout;
    pipelineInfo.descriptorSetLayoutCount = 1;
    pipelineInfo.pushConstantRanges = &pushConstRange;
    pipelineInfo.pushConstantRangeCount = 1;
    pipelineInfo.alphaBlending = true;

    if (!pipeline.Create(buffDevice, pipelineInfo))
    {
        Destroy();
        return false;
    }

    return true;
}

void VulkanUIPass::Destroy()
{
    pipeline.Destroy();
    vertexBuffer.Destroy();
    fontAtlas.Destroy();

    if (device)
    {
        const VkDevice buffDevice = device->GetDevice();

        if (buffDevice)
        {
            if (descPool) vkDestroyDescriptorPool(buffDevice, descPool, nullptr);
            if (descSetLayout) vkDestroyDescriptorSetLayout(buffDevice, descSetLayout, nullptr);
        }
    }

    descSet = VK_NULL_HANDLE;
    descPool = VK_NULL_HANDLE;
    descSetLayout = VK_NULL_HANDLE;

    font = nullptr;
    fontVersion = 0;

    device = nullptr;
}

void VulkanUIPass::Record(const VulkanUIPassRenderInfo& renderInfo)
{
    if (!renderInfo.cmdBuffer || !renderInfo.targetView || !renderInfo.uiData) return;
    if (renderInfo.extent.width == 0 || renderInfo.extent.height == 0) return;

    const UIRenderData& uiData = *renderInfo.uiData;

    if (!uiData.font || uiData.vertices.empty()) return;

    if (font != uiData.font || fontVersion != uiData.fontVersion)
    {
        if (!SetFont(*uiData.font)) return;

        font = uiData.font;
        fontVersion = uiData.fontVersion;
    }

    if (!UpdateVertexBuffer(uiData.vertices)) return;

    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = renderInfo.targetView;
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
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

    const VkBuffer vertexBufferHandle = vertexBuffer.GetBuffer();
    const VkDeviceSize vertexOffset = 0;

    vkCmdBindVertexBuffers(renderInfo.cmdBuffer, 0, 1, &vertexBufferHandle, &vertexOffset);
    vkCmdBindDescriptorSets(renderInfo.cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline.GetPipelineLayout(), 0, 1, &descSet, 0, nullptr);

    UIPushConst pushConst{};
    pushConst.viewportSize = Vector2f((float32)renderInfo.extent.width, (float32)renderInfo.extent.height);

    vkCmdPushConstants(renderInfo.cmdBuffer, pipeline.GetPipelineLayout(),
        VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UIPushConst), &pushConst);

    vkCmdDraw(renderInfo.cmdBuffer, (uint32)uiData.vertices.size(), 1, 0, 0);

    vkCmdEndRendering(renderInfo.cmdBuffer);
}

bool VulkanUIPass::SetFont(const UIFont& _font)
{
    if (!device || !descSet) return false;
    if (_font.GetAtlasPixels().empty()) return false;

    const VkDevice buffDevice = device->GetDevice();
    if (!buffDevice) return false;

    fontAtlas.Destroy();

    TextureConfig fontConfig{};
    fontConfig.mipmaps = false;
    fontConfig.anisotropy = 1.0f;

    VulkanTextureCreateInfo textureInfo{};
    textureInfo.device = device;
    textureInfo.pixels = _font.GetAtlasPixels().data();
    textureInfo.width = _font.GetAtlasWidth();
    textureInfo.height = _font.GetAtlasHeight();
    textureInfo.config = fontConfig;
    textureInfo.format = ImageFormat::R8_UNORM;

    if (!fontAtlas.Create(textureInfo)) return false;

    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = fontAtlas.GetSampler();
    imageInfo.imageView = fontAtlas.GetImageView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet writeSet{};
    writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.dstSet = descSet;
    writeSet.dstBinding = 0;
    writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeSet.descriptorCount = 1;
    writeSet.pImageInfo = &imageInfo;
    writeSet.pNext = nullptr;

    vkUpdateDescriptorSets(buffDevice, 1, &writeSet, 0, nullptr);

    return true;
}

bool VulkanUIPass::UpdateVertexBuffer(std::span<const UIVertex> vertices)
{
    if (!device || vertices.empty()) return false;

    const VkDeviceSize dataSize = (VkDeviceSize)vertices.size() * sizeof(UIVertex);

    if (vertexBuffer.GetSize() < dataSize)
    {
        vertexBuffer.Destroy();

        const VkMemoryPropertyFlags memoryProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        if (!vertexBuffer.Create(device, dataSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, memoryProps))
        {
            return false;
        }
    }

    return vertexBuffer.Upload(vertices.data(), dataSize);
}
#endif
