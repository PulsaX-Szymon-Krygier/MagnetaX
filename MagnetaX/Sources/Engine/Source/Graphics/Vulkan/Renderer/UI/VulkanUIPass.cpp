// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanUIPass.h"

#if MX_GRAPHICS_VULKAN
#include <MX/Generated/Shaders/UI/VulkanShaderUIVert.h>
#include <MX/Generated/Shaders/UI/VulkanShaderUIFrag.h>
#include <Graphics/Vulkan/VulkanDevice.h>
#include "VulkanUIRenderer.h"
#include <cstddef>
#include <algorithm>

namespace
{
    struct UIPushConst
    {
        Vector2f viewportSize;
    };
}

bool VulkanUIPass::Create(const VulkanUIPassCreateInfo& createInfo)
{
    if (!createInfo.device || createInfo.outFormat == VK_FORMAT_UNDEFINED || !createInfo.uiRenderer) return false;

    const VkDevice buffDevice = createInfo.device->GetDevice();
    if (!buffDevice) return false;

    Destroy();

    device = createInfo.device;
    uiRenderer = createInfo.uiRenderer;

    const VkDescriptorSetLayout textureDescSetLayout = uiRenderer->GetTextureDescriptorSetLayout();

    if (!textureDescSetLayout)
    {
        Destroy();
        return false;
    }

    VkVertexInputBindingDescription vertexBinding{};
    vertexBinding.binding = 0;
    vertexBinding.stride = sizeof(UIDrawVertex);
    vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vertexAttribs[3]{};

    vertexAttribs[0].location = 0;
    vertexAttribs[0].binding = 0;
    vertexAttribs[0].format = VK_FORMAT_R32G32_SFLOAT;
    vertexAttribs[0].offset = offsetof(UIDrawVertex, position);

    vertexAttribs[1].location = 1;
    vertexAttribs[1].binding = 0;
    vertexAttribs[1].format = VK_FORMAT_R32G32_SFLOAT;
    vertexAttribs[1].offset = offsetof(UIDrawVertex, uv);

    vertexAttribs[2].location = 2;
    vertexAttribs[2].binding = 0;
    vertexAttribs[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vertexAttribs[2].offset = offsetof(UIDrawVertex, color);

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
    pipelineInfo.descriptorSetLayouts = &textureDescSetLayout;
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
    indexBuffer.Destroy();

    uiRenderer = nullptr;

    device = nullptr;
}

void VulkanUIPass::Record(const VulkanUIPassRenderInfo& renderInfo)
{
    if (!renderInfo.cmdBuffer || !renderInfo.targetView || !uiRenderer) return;
    if (renderInfo.extent.width == 0 || renderInfo.extent.height == 0) return;

    const UIDrawData& drawData = uiRenderer->GetDrawData();

    if (drawData.vertices.empty() || drawData.indices.empty() || drawData.commands.empty()) return;
    if (!UpdateBuffers(drawData)) return;

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

    vkCmdSetViewport(renderInfo.cmdBuffer, 0, 1, &viewport);
    vkCmdBindPipeline(renderInfo.cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipeline());

    UIPushConst pushConst{};
    pushConst.viewportSize = Vector2f((float32)renderInfo.extent.width, (float32)renderInfo.extent.height);

    vkCmdPushConstants(renderInfo.cmdBuffer, pipeline.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UIPushConst), &pushConst);
    
    const VkBuffer vertexBufferHandle = vertexBuffer.GetBuffer();
    const VkBuffer indexBufferHandle = indexBuffer.GetBuffer();
    const VkDeviceSize vertexOffset = 0;

    vkCmdBindVertexBuffers(renderInfo.cmdBuffer, 0, 1, &vertexBufferHandle, &vertexOffset);
    vkCmdBindIndexBuffer(renderInfo.cmdBuffer, indexBufferHandle, 0, VK_INDEX_TYPE_UINT32);

    for (const UIDrawCommand& command : drawData.commands)
    {
        if (command.indexCount == 0 || !command.texture) continue;

        const VkDescriptorSet textureDescSet = uiRenderer->GetTextureDescriptorSet(command.texture);
        if (!textureDescSet) continue;

        const float32 clipMinX = std::max(command.clipMin.x, 0.0f);
        const float32 clipMinY = std::max(command.clipMin.y, 0.0f);
        const float32 clipMaxX = std::min(command.clipMax.x, (float32)renderInfo.extent.width);
        const float32 clipMaxY = std::min(command.clipMax.y, (float32)renderInfo.extent.height);

        if (clipMaxX <= clipMinX || clipMaxY <= clipMinY) continue;

        VkRect2D scissor{};
        scissor.offset = { (int32)clipMinX, (int32)clipMinY };
        scissor.extent = { (uint32)(clipMaxX - clipMinX), (uint32)(clipMaxY - clipMinY) };

        vkCmdSetScissor(renderInfo.cmdBuffer, 0, 1, &scissor);
        vkCmdBindDescriptorSets(renderInfo.cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipelineLayout(), 0, 1, &textureDescSet, 0, nullptr);
        vkCmdDrawIndexed(renderInfo.cmdBuffer, command.indexCount, 1, command.indexOffset, (int32)command.vertexOffset, 0);
    }

    vkCmdEndRendering(renderInfo.cmdBuffer);
}

/* Keeping for reference
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
*/

bool VulkanUIPass::UpdateBuffers(const UIDrawData& drawData)
{
    if (!device || drawData.vertices.empty() || drawData.indices.empty()) return false;

    const VkMemoryPropertyFlags memoryProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    const VkDeviceSize vertexDataSize = (VkDeviceSize)drawData.vertices.size() * sizeof(UIDrawVertex);

    if (vertexBuffer.GetSize() < vertexDataSize)
    {
        vertexBuffer.Destroy();

        if (!vertexBuffer.Create(device, vertexDataSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, memoryProps)) return false;
    }

    if (!vertexBuffer.Upload(drawData.vertices.data(), vertexDataSize)) return false;

    const VkDeviceSize indexDataSize = (VkDeviceSize)drawData.indices.size() * sizeof(uint32);

    if (indexBuffer.GetSize() < indexDataSize)
    {
        indexBuffer.Destroy();

        if (!indexBuffer.Create(device, indexDataSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, memoryProps)) return false;
    }

    return indexBuffer.Upload(drawData.indices.data(), indexDataSize);
}
#endif
