// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanUIRenderer.h"

#if MX_GRAPHICS_VULKAN
#include <Graphics/Vulkan/VulkanDevice.h>
#include <Graphics/Vulkan/Resources/VulkanTexture.h>
#include <Graphics/Vulkan/VulkanInitializers.h>

namespace
{
    constexpr uint32 UI_TEXTURE_CAP = 1024;
}

VulkanUIRenderer::VulkanUIRenderer() = default;
VulkanUIRenderer::~VulkanUIRenderer() = default;

bool VulkanUIRenderer::Create(VulkanDevice* _device)
{
    if (!_device || !_device->GetDevice()) return false;

    Destroy();

    device = _device;

    VkDescriptorSetLayoutBinding textureBinding{};
    textureBinding.binding = 0;
    textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureBinding.descriptorCount = 1;
    textureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    const VkDescriptorSetLayoutCreateInfo layoutInfo = VulkanInitializers::DescriptorSetLayoutCreateInfo(1, &textureBinding);

    if (vkCreateDescriptorSetLayout(device->GetDevice(), &layoutInfo, nullptr, &textureDescSetLayout) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = UI_TEXTURE_CAP;

    VkDescriptorPoolCreateInfo poolInfo = VulkanInitializers::DescriptorPoolCreateInfo(UI_TEXTURE_CAP, 1, &poolSize);
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if (vkCreateDescriptorPool(device->GetDevice(), &poolInfo, nullptr, &textureDescPool) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    return true;
}

void VulkanUIRenderer::Destroy()
{
    for (auto& entry : textures)
    {
        if (entry.second.texture) entry.second.texture->Destroy();
    }
    textures.clear();

    nextTextureID = 1;

    if (device && device->GetDevice())
    {
        if (textureDescPool) vkDestroyDescriptorPool(device->GetDevice(), textureDescPool, nullptr);
        if (textureDescSetLayout) vkDestroyDescriptorSetLayout(device->GetDevice(), textureDescSetLayout, nullptr);
    }

    textureDescPool = VK_NULL_HANDLE;
    textureDescSetLayout = VK_NULL_HANDLE;

    device = nullptr;
}

UITextureHandle VulkanUIRenderer::CreateTexture(const UITextureCreateInfo& createInfo)
{
    if (!device || !device->GetDevice()) return {};
    if (!createInfo.pixels || createInfo.width == 0 || createInfo.height == 0) return {};
    if (createInfo.format == ImageFormat::UNKNOWN) return {};
    if (nextTextureID == 0) return {};

    std::unique_ptr<VulkanTexture> texture = std::make_unique<VulkanTexture>();

    VulkanTextureCreateInfo textureInfo{};
    textureInfo.device = device;
    textureInfo.pixels = createInfo.pixels;
    textureInfo.width = createInfo.width;
    textureInfo.height = createInfo.height;
    textureInfo.format = createInfo.format;
    textureInfo.config.mipmaps = false;
    textureInfo.config.anisotropy = 1.0f;
    textureInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    textureInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    textureInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    if (!texture->Create(textureInfo)) return {};

    VkDescriptorSet descSet = VK_NULL_HANDLE;

    const VkDescriptorSetAllocateInfo allocInfo = VulkanInitializers::DescriptorSetAllocateInfo(textureDescPool, 1, &textureDescSetLayout);

    if (vkAllocateDescriptorSets(device->GetDevice(), &allocInfo, &descSet) != VK_SUCCESS)
    {
        texture->Destroy();
        return {};
    }

    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = texture->GetSampler();
    imageInfo.imageView = texture->GetImageView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet writeSet{};
    writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.dstSet = descSet;
    writeSet.dstBinding = 0;
    writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeSet.descriptorCount = 1;
    writeSet.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(device->GetDevice(), 1, &writeSet, 0, nullptr);

    const uint64 id = nextTextureID++;

    TextureEntry buffEntry{};
    buffEntry.texture = std::move(texture);
    buffEntry.descSet = descSet;

    textures.emplace(id, std::move(buffEntry));

    return UITextureHandle{ id };
}

void VulkanUIRenderer::DestroyTexture(UITextureHandle texture)
{
    if (!texture || !device) return;

    const auto it = textures.find(texture.id);
    if (it == textures.end()) return;

    vkDeviceWaitIdle(device->GetDevice());

    if (it->second.descSet && textureDescPool) vkFreeDescriptorSets(device->GetDevice(), textureDescPool, 1, &it->second.descSet);
    if (it->second.texture) it->second.texture->Destroy();

    textures.erase(it);
}

const VulkanTexture* VulkanUIRenderer::GetTexture(UITextureHandle texture) const
{
    if (!texture) return nullptr;

    const auto it = textures.find(texture.id);
    if (it == textures.end()) return nullptr;

    return it->second.texture.get();
}

VkDescriptorSet VulkanUIRenderer::GetTextureDescriptorSet(UITextureHandle texture) const
{
    if (!texture) return VK_NULL_HANDLE;

    const auto it = textures.find(texture.id);
    if (it == textures.end()) return VK_NULL_HANDLE;

    return it->second.descSet;
}
#endif
