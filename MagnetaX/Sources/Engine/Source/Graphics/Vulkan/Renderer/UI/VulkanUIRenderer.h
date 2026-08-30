// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Graphics/Renderer/UI/AbstractUIRenderer.h>
#include <Graphics/Vulkan/VulkanCommon.h>
#include <memory>
#include <unordered_map>

class VulkanDevice;
class VulkanTexture;

class VulkanUIRenderer final : public AbstractUIRenderer
{
public:
    VulkanUIRenderer();
    ~VulkanUIRenderer();

    bool Create(VulkanDevice* _device);
    void Destroy();

    UITextureHandle CreateTexture(const UITextureCreateInfo& createInfo) override;
    void DestroyTexture(UITextureHandle texture) override;
    const VulkanTexture* GetTexture(UITextureHandle texture) const;

    UIDrawData& GetDrawData() override { return drawData; }
    const UIDrawData& GetDrawData() const { return drawData; }

    VkDescriptorSetLayout GetTextureDescriptorSetLayout() const { return textureDescSetLayout; }
    VkDescriptorSet GetTextureDescriptorSet(UITextureHandle texture) const;

    UITextureHandle RegisterExternalTexture(VkImageView imageView, VkSampler sampler);
    void UnregisterExternalTexture(UITextureHandle texture);

private:
    struct TextureEntry
    {
        std::unique_ptr<VulkanTexture> texture;
        VkDescriptorSet descSet = VK_NULL_HANDLE;
    };

    VulkanDevice* device = nullptr;

    std::unordered_map<uint64, TextureEntry> textures;

    VkDescriptorSetLayout textureDescSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool textureDescPool = VK_NULL_HANDLE;

    uint64 nextTextureID = 1;

    UIDrawData drawData;
};
