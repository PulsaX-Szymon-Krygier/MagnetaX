// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanGBufferBindings.h"

#if MX_GRAPHICS_VULKAN
#include <Graphics/Vulkan/VulkanDevice.h>
#include <Graphics/Vulkan/VulkanInitializers.h>
#include "VulkanGBuffer.h"

bool VulkanGBufferBindings::Create(VulkanDevice* _device, VulkanGBuffer* gBuffer)
{
    if (!_device || !gBuffer) return false;

    const VkDevice buffDevice = _device->GetDevice();
    if (!buffDevice) return false;

    Destroy();

    device = buffDevice;

    VkDescriptorSetLayoutBinding bindings[5]{};

    for (uint32 i = 0; i < 5; ++i)
    {
        bindings[i].binding = i;
        bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[i].descriptorCount = 1;
        bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    const VkDescriptorSetLayoutCreateInfo layoutInfo = VulkanInitializers::DescriptorSetLayoutCreateInfo(5, bindings);

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descSetLayout) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 5;

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

    VkDescriptorImageInfo imageInfos[5]{};

    for (uint32 i = 0; i < 5; ++i)
    {
        imageInfos[i].sampler = sampler;
        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    VkImageView velocityView = gBuffer->GetVelocityImage().GetImageView();

    imageInfos[0].imageView = gBuffer->GetAlbedoImage().GetImageView();
    imageInfos[1].imageView = gBuffer->GetNormalImage().GetImageView();
    imageInfos[2].imageView = gBuffer->GetMaterialImage().GetImageView();
    imageInfos[3].imageView = velocityView ? velocityView : gBuffer->GetAlbedoImage().GetImageView();
    imageInfos[4].imageView = gBuffer->GetDepthImage().GetImageView();
    imageInfos[4].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet writes[5]{};

    for (uint32 i = 0; i < 5; ++i)
    {
        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet = descSet;
        writes[i].dstBinding = i;
        writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writes[i].descriptorCount = 1;
        writes[i].pImageInfo = &imageInfos[i];
    }

    vkUpdateDescriptorSets(device, 5, writes, 0, nullptr);

    return true;
}

void VulkanGBufferBindings::Destroy()
{
    if (device)
    {
        if (descPool) vkDestroyDescriptorPool(device, descPool, nullptr);
        if (sampler) vkDestroySampler(device, sampler, nullptr);
        if (descSetLayout) vkDestroyDescriptorSetLayout(device, descSetLayout, nullptr);
    }

    descSet = VK_NULL_HANDLE;
    descPool = VK_NULL_HANDLE;
    descSetLayout = VK_NULL_HANDLE;
    sampler = VK_NULL_HANDLE;

    device = VK_NULL_HANDLE;
}
#endif
