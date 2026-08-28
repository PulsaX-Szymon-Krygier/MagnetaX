// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/Math/Matrix.h>
#include <Graphics/Vulkan/Resources/VulkanBuffer.h>
#include "VulkanGBufferBindings.h"
#include "../VulkanPass.h"
#include "../VulkanPipeline.h"

class VulkanGBuffer;

struct RenderSceneData;
struct ShadowFrameData;

struct VulkanLightingPassCreateInfo : VulkanPassCreateInfo
{
    VulkanGBuffer* gBuffer = nullptr;

    VkFormat outFormat = VK_FORMAT_UNDEFINED;

    VkImageView directionalShadowView = VK_NULL_HANDLE;
    VkSampler directionalShadowSampler = VK_NULL_HANDLE;

    VkImageView spotShadowView = VK_NULL_HANDLE;
    VkSampler spotShadowSampler = VK_NULL_HANDLE;
};

struct VulkanLightingPassRenderInfo : VulkanPassRenderInfo
{
    VkImageView targetView = VK_NULL_HANDLE;
    VkExtent2D extent{};

    const RenderSceneData* sceneData = nullptr;
    const ShadowFrameData* shadowData = nullptr;

    VkImageView specularEnvView = VK_NULL_HANDLE;
    VkSampler specularEnvSampler = VK_NULL_HANDLE;

    VkImageView brdfLUTView = VK_NULL_HANDLE;
    VkSampler brdfLUTSampler = VK_NULL_HANDLE;
};

class VulkanLightingPass
{
public:
    VulkanLightingPass() = default;
    VulkanLightingPass(const VulkanLightingPass&) = delete;
    VulkanLightingPass& operator=(const VulkanLightingPass&) = delete;

    bool Create(const VulkanLightingPassCreateInfo& createInfo);
    void Destroy();

    void Record(const VulkanLightingPassRenderInfo& renderInfo);

private:
    VkDevice device = VK_NULL_HANDLE;

    VulkanPipeline pipeline;
    VulkanGBufferBindings gBufferBindings;

    VulkanBuffer frameDataBuffer;
    VulkanBuffer lightBuffer;

    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
    VkDescriptorSet descSet = VK_NULL_HANDLE;
};
