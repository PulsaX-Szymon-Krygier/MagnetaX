// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Graphics/Renderer/UI/AbstractUIRenderer.h>
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

private:
    VulkanDevice* device = nullptr;

    std::unordered_map<uint64, std::unique_ptr<VulkanTexture>> textures;

    uint64 nextTextureID = 1;
};
