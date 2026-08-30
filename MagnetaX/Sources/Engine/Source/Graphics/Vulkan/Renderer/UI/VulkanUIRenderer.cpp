// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanUIRenderer.h"

#if MX_GRAPHICS_VULKAN
#include <Graphics/Vulkan/VulkanDevice.h>
#include <Graphics/Vulkan/Resources/VulkanTexture.h>

VulkanUIRenderer::VulkanUIRenderer() = default;
VulkanUIRenderer::~VulkanUIRenderer() = default;

bool VulkanUIRenderer::Create(VulkanDevice* _device)
{
    if (!_device || !_device->GetDevice()) return false;

    Destroy();

    device = _device;

    return true;
}

void VulkanUIRenderer::Destroy()
{
    for (auto& entry : textures)
    {
        if (entry.second) entry.second->Destroy();
    }
    textures.clear();

    nextTextureID = 1;
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

    if (!texture->Create(textureInfo))
    {
        return {};
    }

    const uint64 id = nextTextureID++;

    textures.emplace(id, std::move(texture));

    return UITextureHandle{ id };
}

void VulkanUIRenderer::DestroyTexture(UITextureHandle texture)
{
    if (!texture) return;

    const auto it = textures.find(texture.id);
    if (it == textures.end()) return;

    if (it->second) it->second->Destroy();

    textures.erase(it);
}
#endif
