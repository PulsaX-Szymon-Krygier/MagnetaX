// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Graphics/GraphicsDebugView.h>
#include <MX/Graphics/Renderer/RendererConfig.h>
#include <Graphics/Vulkan/VulkanCommon.h>
#include <Graphics/Vulkan/Resources/VulkanImage.h>
#include "Deferred/VulkanGBufferDebugPass.h"
#include "Deferred/VulkanGBufferPass.h"
#include "Deferred/VulkanLightingPass.h"
#include "PostFX/VulkanPostFXPass.h"
#include "Shadow/VulkanShadowDepthPass.h"
#include "UI/VulkanUIPass.h"
#include "VulkanCommandPool.h"
#include "VulkanDrawItem.h"
#include <span>
#include <vector>

class VulkanPresentContext;
struct RenderSceneData;
struct UIRenderData;

enum class VulkanFrameResult
{
    SUCCESS,
    RECREATE,
    FAILED
};

struct VulkanRendererCreateInfo
{
    VulkanDevice* device = nullptr;
    VulkanPresentContext* presentContext = nullptr;

    VkDescriptorSetLayout materialDescSetLayout = VK_NULL_HANDLE;

    RendererConfig config{};
};

class VulkanRenderer
{
public:
    VulkanRenderer() = default;
    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;

    bool Create(const VulkanRendererCreateInfo& createInfo);
    void Destroy();

    VulkanFrameResult DrawFrame(std::span<const VulkanDrawItem> drawItems, const RenderSceneData& sceneData, const UIRenderData& uiData);

    void SetDebugView(GraphicsDebugView view) { debugView = view; }

private:
    VulkanDevice* device = nullptr;
    VulkanPresentContext* presentContext = nullptr;

    RendererConfig config{};

    VulkanCommandPool commandPool;
    VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;

    VulkanShadowDepthPass directionalShadowPass;
    VulkanShadowDepthPass spotShadowPass;

    VulkanGBufferPass gBufferPass;

    VulkanImage sceneColor;
    VkImageLayout sceneColorLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VulkanLightingPass lightingPass;
    VulkanGBufferDebugPass gBufferDebugPass;

    VulkanPostFXPass postFXPass;
    VulkanUIPass uiPass;

    VkSemaphore imageAvailable = VK_NULL_HANDLE;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    VkFence inFlight = VK_NULL_HANDLE;

    std::vector<VkImageLayout> swapchainImageLayouts;

    GraphicsDebugView debugView = GraphicsDebugView::FINAL;
};
