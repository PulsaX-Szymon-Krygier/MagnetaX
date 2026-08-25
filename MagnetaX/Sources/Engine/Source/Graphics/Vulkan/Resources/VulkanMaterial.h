// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/Math/Vector.h>
#include <Graphics/Vulkan/VulkanCommon.h>
#include "VulkanBuffer.h"

struct VulkanMaterialCreateInfo
{
    VulkanDevice* device = nullptr;

    VkDescriptorPool descPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;

    VulkanTexture* baseColorTexture = nullptr;

    Vector2f uvScale{};
    Vector4f baseColor{};

    float32 roughness = 0.0f;
    float32 metallic = 0.0f;
    float32 ambientOcclusion = 0.0f;
};

class VulkanMaterial
{
public:
    VulkanMaterial() = default;
    VulkanMaterial(const VulkanMaterial&) = delete;
    VulkanMaterial& operator=(const VulkanMaterial&) = delete;

    bool Create(const VulkanMaterialCreateInfo& createInfo);
    void Destroy();

    VkDescriptorSet GetDescriptorSet() const { return descSet; }

private:
    VulkanBuffer materialBuffer;

    VkDescriptorSet descSet = VK_NULL_HANDLE;
};
