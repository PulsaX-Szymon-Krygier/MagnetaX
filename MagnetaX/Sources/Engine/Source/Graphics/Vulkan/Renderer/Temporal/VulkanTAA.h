// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <Graphics/Vulkan/Resources/VulkanImage.h>
#include "VulkanCamVelocityPass.h"
#include "VulkanTAAPass.h"
#include "VulkanTAAPreparePass.h"
#include <array>

struct VulkanTAACreateInfo
{
    VulkanDevice* device = nullptr;
    VkExtent2D extent{};

    VulkanImage* currentColor = nullptr;
    const VulkanImage* velocityImage = nullptr;
    const VulkanImage* depthImage = nullptr;
};

struct VulkanTAAResolveInfo
{
    VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;

    Vector2f jitterUV{};

    float32 feedbackMin = 0.88f;
    float32 feedbackMax = 0.97f;

    float32 nearPlane = 0.1f;
    float32 farPlane = 1000.0f;
    Vector2f projScale{};

    Matrix4f currentViewProj = Matrix4f::Identity();
    Matrix4f previousInvViewProj = Matrix4f::Identity();
};

class VulkanTAA
{
public:
    VulkanTAA() = default;

    bool Create(const VulkanTAACreateInfo& createInfo);
    void Destroy();

    void RecordCameraVelocity(VkCommandBuffer cmdBuffer, const Matrix4f& invViewProj, const Matrix4f& prevViewProj);

    VkImageView Resolve(const VulkanTAAResolveInfo& resolveInfo);

    void ResetHistory();

    Vector2f GetProjectionJitter() const;

    const VulkanImage& GetHistoryImage(uint32 index) const { return history[index]; }
    bool IsHistoryValid() const { return historyValid; }
    

private:
    std::array<VulkanImage, 2> history;
    std::array<VkImageLayout, 2> historyLayouts{ VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED };

    std::array<VulkanImage, 2> supportMean;
    std::array<VkImageLayout, 2> supportMeanLayouts{ VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED };

    std::array<VulkanImage, 2> supportSigma;
    std::array<VkImageLayout, 2> supportSigmaLayouts{ VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED };

    std::array<VulkanImage, 2> depthHistory;
    std::array<VkImageLayout, 2> depthHistoryLayouts{ VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED };

    VulkanCamVelocityPass camVelocityPass;
    VulkanTAAPass taaPass;
    VulkanTAAPreparePass preparePass;

    VkExtent2D extent{};

    uint64 frameIndex = 0;
    uint32 historyReadIndex = 0;
    bool historyValid = false;

    Vector2f prevJitterUV{};

    const VulkanImage* velocityImage = nullptr;
    const VulkanImage* depthImage = nullptr;

    VulkanImage dilatedDepth;
    VkImageLayout dilatedDepthLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VulkanImage dilatedVelocity;
    VkImageLayout dilatedVelocityLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VulkanImage reconstructedPrevDepth;
    VkImageLayout reconstructedPrevDepthLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    void RecordPrepare(VkCommandBuffer cmdBuffer, const Vector2f& jitterUV);
};
