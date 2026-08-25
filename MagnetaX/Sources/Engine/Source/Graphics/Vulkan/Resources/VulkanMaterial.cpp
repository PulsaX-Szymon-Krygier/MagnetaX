// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanMaterial.h"

#if MX_GRAPHICS_VULKAN
#include "VulkanTexture.h"
#include "../VulkanDevice.h"
#include "../VulkanInitializers.h"

namespace
{
    struct VulkanMaterialData
    {
        Vector4f baseColor;
        Vector4f properties;
        Vector2f uvScale;
    };
}

bool VulkanMaterial::Create(const VulkanMaterialCreateInfo& createInfo)
{
    if (!createInfo.device || !createInfo.descPool || !createInfo.descSetLayout || !createInfo.baseColorTexture) return false;

    const VkDevice buffDevice = createInfo.device->GetDevice();
    if (!buffDevice) return false;

    Destroy();

    const VkMemoryPropertyFlags memoryProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    if (!materialBuffer.Create(createInfo.device, sizeof(VulkanMaterialData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, memoryProps))
    {
        Destroy();
        return false;
    }

    VulkanMaterialData materialData{};
    materialData.baseColor = createInfo.baseColor;
    materialData.properties = Vector4f(createInfo.roughness, createInfo.metallic, createInfo.ambientOcclusion, 0.0f);
    materialData.uvScale = createInfo.uvScale;

    if (!materialBuffer.Upload(&materialData, sizeof(VulkanMaterialData)))
    {
        Destroy();
        return false;
    }

    const VkDescriptorSetAllocateInfo allocInfo = VulkanInitializers::DescriptorSetAllocateInfo(createInfo.descPool, 1, &createInfo.descSetLayout);

    if (vkAllocateDescriptorSets(buffDevice, &allocInfo, &descSet) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = createInfo.baseColorTexture->GetSampler();
    imageInfo.imageView = createInfo.baseColorTexture->GetImageView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = materialBuffer.GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(VulkanMaterialData);

    VkWriteDescriptorSet writes[2]{};

    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = descSet;
    writes[0].dstBinding = 0;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[0].descriptorCount = 1;
    writes[0].pImageInfo = &imageInfo;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = descSet;
    writes[1].dstBinding = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[1].descriptorCount = 1;
    writes[1].pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(buffDevice, 2, writes, 0, nullptr);

    return true;
}

void VulkanMaterial::Destroy()
{
    materialBuffer.Destroy();

    descSet = VK_NULL_HANDLE;
}
#endif
