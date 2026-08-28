// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Assets/AssetHandle.h>
#include <MX/Graphics/AbstractGraphicsSystem.h>
#include <Graphics/Vulkan/Present/VulkanSurface.h>
#include <Graphics/Vulkan/Renderer/Environment/VulkanEnvironment.h>
#include <MX/Assets/Texture/TextureAsset.h>
#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanRenderContext.h"
#include <memory>
#include <unordered_map>

class MaterialAsset;
class MeshAsset;
class VulkanMaterial;
class VulkanMesh;
class VulkanTexture;

class VulkanGraphicsSystem final : public AbstractGraphicsSystem
{
public:
    VulkanGraphicsSystem();
    ~VulkanGraphicsSystem() override;

    bool Create(const SurfaceHost& _surfaceHost) override;
    void Destroy() override;

    bool RecreateRenderer(const Size2i& surfaceSize) override;

    void RenderScene(Scene* scene, AssetManager* assetManager, const UIRenderData& uiData) override;

    void SetDebugView(GraphicsDebugView view) override;

    const GraphicsDeviceInfo& GetDeviceInfo() const override;

private:
    const SurfaceHost* surfaceHost = nullptr;

    VulkanDevice device;

    VulkanInstance instance;
    VulkanSurface surface;
    VulkanRenderContext renderContext;

    std::unordered_map<uint64, std::unique_ptr<VulkanMesh>> meshes;
    std::unordered_map<uint64, std::unique_ptr<VulkanTexture>> textures;
    std::unordered_map<uint64, std::unique_ptr<VulkanMaterial>> materials;

    std::unique_ptr<VulkanTexture> fallbackTexture;
    std::unique_ptr<VulkanMaterial> fallbackMaterial;

    VkDescriptorSetLayout materialDescSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool materialDescPool = VK_NULL_HANDLE;

    GraphicsDebugView debugView = GraphicsDebugView::FINAL;

    VulkanEnvironment environment;
    uint64 environmentMapAssetID = 0;

    void UpdateEnvironment(AssetHandle<TextureAsset> environmentMap, AssetManager* assetManager);

    VulkanTexture* GetOrCreateTexture(uint64 assetID, TextureAsset* textureAsset);
    VulkanMaterial* GetOrCreateMaterial(uint64 assetID, MaterialAsset* materialAsset, AssetManager* assetManager);
    VulkanMesh* GetOrCreateMesh(uint64 assetID, MeshAsset* meshAsset);
};
