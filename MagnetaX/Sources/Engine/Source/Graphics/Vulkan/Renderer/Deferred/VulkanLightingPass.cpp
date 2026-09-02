// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanLightingPass.h"
#include <MX/Core/Math/Vector.h>
#include <MX/Generated/Shaders/Common/VulkanShaderFullscreenVert.h>
#include <MX/Generated/Shaders/Deferred/VulkanShaderLightingFrag.h>
#include <Graphics/Renderer/Scene/RenderSceneData.h>
#include <Graphics/Renderer/Shadow/ShadowFrameData.h>
#include <Graphics/Vulkan/VulkanDevice.h>
#include <Graphics/Vulkan/VulkanInitializers.h>
#include "VulkanGBuffer.h"
#include <vector>

#if MX_GRAPHICS_VULKAN
namespace
{
    constexpr uint32 MAX_LIGHTS = 1000;

    struct LightingFrameData
    {
        Matrix4f view;
        Matrix4f viewProjectionInversed;

        Vector4f cameraPosition;

        Matrix4f directionalShadowViewProjs[MX_GRAPHICS_DIRECTIONAL_SHADOW_CASCADE_COUNT];

        Vector4f directionalShadowSplits;
        Vector4f directionalShadowBiases;
        Vector4f directionalShadowBlendWidths;

        Vector4f ambientColorIntensity;
        Vector4f lightInfo;
        Vector4f clearColor;

        Matrix4f spotShadowViewProj;
    };

    struct LightingLightData
    {
        Vector4f positionRange;
        Vector4f direction;
        Vector4f colorIntensity;
        Vector4f params;
    };
}

bool VulkanLightingPass::Create(const VulkanLightingPassCreateInfo& createInfo)
{
    if (!createInfo.device || !createInfo.gBuffer) return false;
    if (createInfo.outFormat == VK_FORMAT_UNDEFINED) return false;
    if (!createInfo.directionalShadowView || !createInfo.directionalShadowSampler) return false;
    if (!createInfo.spotShadowView || !createInfo.spotShadowSampler) return false;

    const VkDevice buffDevice = createInfo.device->GetDevice();
    if (!buffDevice) return false;

    Destroy();

    device = buffDevice;

    if (!gBufferBindings.Create(createInfo.device, createInfo.gBuffer))
    {
        Destroy();
        return false;
    }

    VkDescriptorSetLayoutBinding bindings[6]{};

    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    bindings[2].binding = 2;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[2].pImmutableSamplers = &createInfo.directionalShadowSampler;

    bindings[3].binding = 3;
    bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[3].descriptorCount = 1;
    bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[3].pImmutableSamplers = &createInfo.spotShadowSampler;

    bindings[4].binding = 4;
    bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[4].descriptorCount = 1;
    bindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    bindings[5].binding = 5;
    bindings[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[5].descriptorCount = 1;
    bindings[5].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    const VkDescriptorSetLayoutCreateInfo layoutInfo = VulkanInitializers::DescriptorSetLayoutCreateInfo(6, bindings);

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descSetLayout) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    const VkMemoryPropertyFlags memoryProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    if (!frameDataBuffer.Create(createInfo.device, sizeof(LightingFrameData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, memoryProps))
    {
        Destroy();
        return false;
    }

    if (!lightBuffer.Create(createInfo.device, sizeof(LightingLightData) * MAX_LIGHTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, memoryProps))
    {
        Destroy();
        return false;
    }

    VkDescriptorPoolSize poolSizes[3]{};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = 1;

    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[2].descriptorCount = 4;

    const VkDescriptorPoolCreateInfo poolInfo = VulkanInitializers::DescriptorPoolCreateInfo(1, 3, poolSizes);

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

    VkDescriptorBufferInfo frameBufferInfo{};
    frameBufferInfo.buffer = frameDataBuffer.GetBuffer();
    frameBufferInfo.offset = 0;
    frameBufferInfo.range = sizeof(LightingFrameData);

    VkDescriptorBufferInfo lightBufferInfo{};
    lightBufferInfo.buffer = lightBuffer.GetBuffer();
    lightBufferInfo.offset = 0;
    lightBufferInfo.range = sizeof(LightingLightData) * MAX_LIGHTS;

    VkDescriptorImageInfo directionalShadowInfo{};
    directionalShadowInfo.imageView = createInfo.directionalShadowView;
    directionalShadowInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkDescriptorImageInfo spotShadowInfo{};
    spotShadowInfo.imageView = createInfo.spotShadowView;
    spotShadowInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet writes[4]{};

    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = descSet;
    writes[0].dstBinding = 0;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[0].descriptorCount = 1;
    writes[0].pBufferInfo = &frameBufferInfo;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = descSet;
    writes[1].dstBinding = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writes[1].descriptorCount = 1;
    writes[1].pBufferInfo = &lightBufferInfo;

    writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[2].dstSet = descSet;
    writes[2].dstBinding = 2;
    writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[2].descriptorCount = 1;
    writes[2].pImageInfo = &directionalShadowInfo;

    writes[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[3].dstSet = descSet;
    writes[3].dstBinding = 3;
    writes[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[3].descriptorCount = 1;
    writes[3].pImageInfo = &spotShadowInfo;

    vkUpdateDescriptorSets(device, 4, writes, 0, nullptr);

    const VkDescriptorSetLayout descSetLayouts[2] =
    {
        gBufferBindings.GetDescriptorSetLayout(),
        descSetLayout
    };

    VulkanPipelineCreateInfo pipelineInfo{};
    pipelineInfo.vertexShader = MX_GRAPHICS_VULKAN_SHADER_FULLSCREEN_VERT;
    pipelineInfo.vertexShaderSize = MX_GRAPHICS_VULKAN_SHADER_FULLSCREEN_VERT_SIZE;
    pipelineInfo.fragmentShader = MX_GRAPHICS_VULKAN_SHADER_LIGHTING_FRAG;
    pipelineInfo.fragmentShaderSize = MX_GRAPHICS_VULKAN_SHADER_LIGHTING_FRAG_SIZE;
    pipelineInfo.colorFormats = &createInfo.outFormat;
    pipelineInfo.colorFormatCount = 1;
    pipelineInfo.descriptorSetLayouts = descSetLayouts;
    pipelineInfo.descriptorSetLayoutCount = 2;

    if (!pipeline.Create(device, pipelineInfo))
    {
        Destroy();
        return false;
    }

    return true;
}

void VulkanLightingPass::Destroy()
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

    lightBuffer.Destroy();
    frameDataBuffer.Destroy();

    gBufferBindings.Destroy();

    device = VK_NULL_HANDLE;
}

void VulkanLightingPass::Record(const VulkanLightingPassRenderInfo& renderInfo)
{
    if (!renderInfo.cmdBuffer || !renderInfo.targetView) return;
    if (!renderInfo.sceneData || !renderInfo.shadowData) return;
    if (renderInfo.extent.width == 0 || renderInfo.extent.height == 0) return;

    const RenderSceneData& sceneData = *renderInfo.sceneData;
    const ShadowFrameData& shadowData = *renderInfo.shadowData;

    LightingFrameData frameData{};
    frameData.view = sceneData.view.Transposed();
    //frameData.viewProjectionInversed = sceneData.viewProjectionInversed.Transposed();
    frameData.viewProjectionInversed = sceneData.jitteredViewProjectionInversed.Transposed();
    frameData.cameraPosition = Vector4f(sceneData.cameraPosition, 1.0f);

    for (uint32 i = 0; i < MX_GRAPHICS_DIRECTIONAL_SHADOW_CASCADE_COUNT; ++i)
    {
        frameData.directionalShadowViewProjs[i] = shadowData.directional.viewProjs[i].Transposed();
    }

    frameData.directionalShadowSplits = Vector4f(
        shadowData.directional.splits[0], shadowData.directional.splits[1],
        shadowData.directional.splits[2], shadowData.directional.splits[3]
    );

    frameData.directionalShadowBiases = Vector4f(
        shadowData.directional.biases[0], shadowData.directional.biases[1],
        shadowData.directional.biases[2], shadowData.directional.biases[3]
    );

    frameData.directionalShadowBlendWidths = Vector4f(
        shadowData.directional.blendWidths[0], shadowData.directional.blendWidths[1],
        shadowData.directional.blendWidths[2], shadowData.directional.blendWidths[3]
    );

    frameData.ambientColorIntensity = Vector4f(sceneData.ambientLightColor, sceneData.ambientLightIntensity);
    frameData.lightInfo = {};
    frameData.clearColor = Vector4f(sceneData.backgroundColor, 1.0f);
    frameData.spotShadowViewProj = shadowData.spot.viewProj.Transposed();

    std::vector<LightingLightData> lights;
    lights.reserve(std::min<usize>(sceneData.lights.size(), MAX_LIGHTS));

    for (uint32 i = 0; i < sceneData.lights.size() && lights.size() < MAX_LIGHTS; ++i)
    {
        const RenderLight& light = sceneData.lights[i];

        bool hasShadow = false;

        if (light.type == LightType::DIRECTIONAL)
        {
            hasShadow = shadowData.directional.lightIndex == (int32)i;
        }
        else if (light.type == LightType::SPOT)
        {
            hasShadow = shadowData.spot.lightIndex == (int32)i;
        }

        LightingLightData lightData{};
        lightData.positionRange = Vector4f(light.position, light.range);
        lightData.direction = Vector4f(light.direction, 0.0f);
        lightData.colorIntensity = Vector4f(light.color, light.intensity);
        lightData.params = Vector4f(light.innerConeAngle, light.outerConeAngle, (float32)light.type, hasShadow ? 1.0f : 0.0f);

        lights.push_back(lightData);
    }

    frameData.lightInfo = Vector4f((float32)lights.size(), 0.0f, 0.0f, 0.0f);

    if (!frameDataBuffer.Upload(&frameData, sizeof(LightingFrameData))) return;

    if (!lights.empty())
    {
        if (!lightBuffer.Upload(lights.data(), sizeof(LightingLightData) * lights.size())) return;
    }

    if (renderInfo.specularEnvView && renderInfo.specularEnvSampler)
    {
        VkDescriptorImageInfo specularEnvInfo{};
        specularEnvInfo.sampler = renderInfo.specularEnvSampler;
        specularEnvInfo.imageView = renderInfo.specularEnvView;
        specularEnvInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet specularEnvWrite{};
        specularEnvWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        specularEnvWrite.dstSet = descSet;
        specularEnvWrite.dstBinding = 4;
        specularEnvWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        specularEnvWrite.descriptorCount = 1;
        specularEnvWrite.pImageInfo = &specularEnvInfo;

        vkUpdateDescriptorSets(device, 1, &specularEnvWrite, 0, nullptr);
    }

    if (renderInfo.brdfLUTView && renderInfo.brdfLUTSampler)
    {
        VkDescriptorImageInfo brdfLUTInfo{};
        brdfLUTInfo.sampler = renderInfo.brdfLUTSampler;
        brdfLUTInfo.imageView = renderInfo.brdfLUTView;
        brdfLUTInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet brdfLUTWrite{};
        brdfLUTWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        brdfLUTWrite.dstSet = descSet;
        brdfLUTWrite.dstBinding = 5;
        brdfLUTWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        brdfLUTWrite.descriptorCount = 1;
        brdfLUTWrite.pImageInfo = &brdfLUTInfo;

        vkUpdateDescriptorSets(device, 1, &brdfLUTWrite, 0, nullptr);
    }

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

    const VkDescriptorSet descSets[2] =
    {
        gBufferBindings.GetDescriptorSet(),
        descSet
    };

    vkCmdBindDescriptorSets(renderInfo.cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetPipelineLayout(),
        0, 2, descSets, 0, nullptr);

    vkCmdDraw(renderInfo.cmdBuffer, 3, 1, 0, 0);

    vkCmdEndRendering(renderInfo.cmdBuffer);
}
#endif
