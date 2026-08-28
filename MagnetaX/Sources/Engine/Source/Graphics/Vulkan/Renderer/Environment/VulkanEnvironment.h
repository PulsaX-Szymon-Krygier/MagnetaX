// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Graphics/Vulkan/Resources/VulkanImage.h>
#include "VulkanBRDFLUTPass.h"
#include "VulkanEquirectPass.h"
#include "VulkanSpecularEnvPass.h"

struct VulkanEnvironmentSourceInfo
{
    const uint8* pixels = nullptr;
    uint32 width = 0;
    uint32 height = 0;
    ImageFormat format = ImageFormat::UNKNOWN;
};

struct VulkanEnvironmentRenderData
{
    VkImageView environmentView = VK_NULL_HANDLE;
    VkSampler environmentSampler = VK_NULL_HANDLE;

    VkImageView specularView = VK_NULL_HANDLE;
    VkSampler specularSampler = VK_NULL_HANDLE;

    VkImageView brdfLUTView = VK_NULL_HANDLE;
    VkSampler brdfLUTSampler = VK_NULL_HANDLE;
};

struct VulkanEnvironmentCreateInfo
{
    VulkanDevice* device = nullptr;
};

class VulkanEnvironment
{
public:
    VulkanEnvironment() = default;
    VulkanEnvironment(const VulkanEnvironment&) = delete;
    VulkanEnvironment& operator=(const VulkanEnvironment&) = delete;

    bool Create(const VulkanEnvironmentCreateInfo& createInfo);
    void Destroy();

    bool SetSource(const VulkanEnvironmentSourceInfo& sourceInfo);
    void ClearSource();

    VulkanEnvironmentRenderData GetRenderData() const;

private:
    VulkanDevice* device = nullptr;

    VulkanEquirectPass equirectPass;
    VulkanSpecularEnvPass specularEnvPass;
    VulkanBRDFLUTPass brdfLUTPass;

    VulkanImage environmentCubemap;
    VkSampler environmentSampler = VK_NULL_HANDLE;

    VulkanImage specularCubemap;
    VkSampler specularSampler = VK_NULL_HANDLE;

    VulkanImage brdfLUT;
    VkSampler brdfLUTSampler = VK_NULL_HANDLE;

    VulkanImage fallbackSpecularCubemap;
    VkSampler fallbackSpecularSampler = VK_NULL_HANDLE;

    bool GenerateBRDFLUT();
    bool CreateFallbackSpecular();

    void DestroyEnvironmentResources();
    void DestroyBRDFLUT();
    void DestroyFallbackSpecular();
};
