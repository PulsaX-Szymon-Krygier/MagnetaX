// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanGraphicsSystem.h"

#if MX_GRAPHICS_VULKAN
#include <MX/Assets/AssetManager.h>
#include <MX/Assets/Material/MaterialAsset.h>
#include <MX/Assets/Mesh/MeshAsset.h>
#include <MX/Assets/Texture/TextureAsset.h>
#include <MX/Graphics/Renderer/UI/UIRenderData.h>
#include <MX/Window/SurfaceHost.h>
#include <Graphics/Renderer/Scene/RenderSceneBuilder.h>
#include <Graphics/Vulkan/Resources/VulkanMaterial.h>
#include <Graphics/Vulkan/Resources/VulkanMesh.h>
#include <Graphics/Vulkan/Resources/VulkanTexture.h>
#include "VulkanInitializers.h"
#include <vector>

VulkanGraphicsSystem::VulkanGraphicsSystem() = default;
VulkanGraphicsSystem::~VulkanGraphicsSystem() = default;

bool VulkanGraphicsSystem::Create(const SurfaceHost& _surfaceHost)
{
    if (instance.GetInstance() || device.GetDevice()) return false;

    surfaceHost = &_surfaceHost;

    const NativeWindowHandle nativeWindowHandle = _surfaceHost.GetNativeHandle();
    const Size2i surfaceSize = _surfaceHost.GetSurfaceSize();

    if (!instance.Create())
    {
        Destroy();
        return false;
    }

    if (!surface.Create(instance.GetInstance(), nativeWindowHandle))
    {
        Destroy();
        return false;
    }

    if (!device.Create(instance.GetInstance(), surface.GetSurface()))
    {
        Destroy();
        return false;
    }

    const VkDevice buffDevice = device.GetDevice();

    if (!buffDevice)
    {
        Destroy();
        return false;
    }

    VkDescriptorSetLayoutBinding bindings[2]{};

    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    const VkDescriptorSetLayoutCreateInfo layoutInfo = VulkanInitializers::DescriptorSetLayoutCreateInfo(2, bindings);

    if (vkCreateDescriptorSetLayout(buffDevice, &layoutInfo, nullptr, &materialDescSetLayout) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    VkDescriptorPoolSize poolSizes[2]{};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[0].descriptorCount = 1024;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = 1024;

    const VkDescriptorPoolCreateInfo poolInfo = VulkanInitializers::DescriptorPoolCreateInfo(1024, 2, poolSizes);

    if (vkCreateDescriptorPool(buffDevice, &poolInfo, nullptr, &materialDescPool) != VK_SUCCESS)
    {
        Destroy();
        return false;
    }

    const uint8 whitePixel[] = { 255, 255, 255, 255 };

    fallbackTexture = std::make_unique<VulkanTexture>();

    VulkanTextureCreateInfo textureInfo{};
    textureInfo.device = &device;
    textureInfo.pixels = whitePixel;
    textureInfo.width = 1;
    textureInfo.height = 1;
    textureInfo.format = ImageFormat::RGBA8_SRGB;
    textureInfo.config = config.texture;

    if (!fallbackTexture->Create(textureInfo))
    {
        Destroy();
        return false;
    }

    fallbackMaterial = std::make_unique<VulkanMaterial>();

    VulkanMaterialCreateInfo materialInfo{};
    materialInfo.device = &device;
    materialInfo.descPool = materialDescPool;
    materialInfo.descSetLayout = materialDescSetLayout;
    materialInfo.baseColorTexture = fallbackTexture.get();
    materialInfo.uvScale = Vector2f(1.0f);
    materialInfo.baseColor = Vector4f(0.8f, 0.8f, 0.8f, 1.0f);
    materialInfo.roughness = 0.5f;
    materialInfo.metallic = 0.0f;
    materialInfo.ambientOcclusion = 1.0f;

    if (!fallbackMaterial->Create(materialInfo))
    {
        Destroy();
        return false;
    }

    if (!RecreateRenderer(surfaceSize))
    {
        Destroy();
        return false;
    }

    return true;
}

void VulkanGraphicsSystem::Destroy()
{
    renderContext.Destroy();

    for (auto& material : materials)
    {
        if (material.second) material.second->Destroy();
    }

    materials.clear();

    if (fallbackMaterial) fallbackMaterial->Destroy();
    fallbackMaterial.reset();

    for (auto& texture : textures)
    {
        if (texture.second) texture.second->Destroy();
    }

    textures.clear();

    if (fallbackTexture) fallbackTexture->Destroy();
    fallbackTexture.reset();

    for (auto& mesh : meshes)
    {
        if (mesh.second) mesh.second->Destroy();
    }

    meshes.clear();

    const VkDevice buffDevice = device.GetDevice();

    if (buffDevice)
    {
        if (materialDescPool) vkDestroyDescriptorPool(buffDevice, materialDescPool, nullptr);
        if (materialDescSetLayout) vkDestroyDescriptorSetLayout(buffDevice, materialDescSetLayout, nullptr);
    }

    materialDescPool = VK_NULL_HANDLE;
    materialDescSetLayout = VK_NULL_HANDLE;

    device.Destroy();
    surface.Destroy();
    instance.Destroy();

    surfaceHost = nullptr;
    debugView = GraphicsDebugView::FINAL;
}

bool VulkanGraphicsSystem::RecreateRenderer(const Size2i& surfaceSize)
{
    if (!device.GetDevice() || !surface.GetSurface()) return false;

    renderContext.Destroy();

    if (surfaceSize.width == 0 || surfaceSize.height == 0) return true;

    VulkanRenderContextCreateInfo renderInfo{};
    renderInfo.device = &device;
    renderInfo.surface = surface.GetSurface();
    renderInfo.extent = { surfaceSize.width, surfaceSize.height };
    renderInfo.materialDescSetLayout = materialDescSetLayout;
    renderInfo.config = config.renderer;

    if (!renderContext.Create(renderInfo))
    {
        renderContext.Destroy();
        return false;
    }

    renderContext.GetRenderer().SetDebugView(debugView);

    return true;
}

void VulkanGraphicsSystem::RenderScene(Scene* scene, AssetManager* assetManager, const UIRenderData& uiData)
{
    if (!renderContext.IsValid()) return;

    VulkanRenderer& renderer = renderContext.GetRenderer();
    const VkExtent2D extent = renderContext.GetPresentContext().GetSwapchain().GetExtent();

    if (extent.width == 0 || extent.height == 0) return;

    const Size2i renderSize(extent.width, extent.height);
    const RenderSceneData sceneData = BuildRenderSceneData(scene, renderSize);

    std::vector<VulkanDrawItem> drawItems;
    drawItems.reserve(sceneData.objects.size());

    if (assetManager)
    {
        for (const RenderObject& object : sceneData.objects)
        {
            if (!object.mesh) continue;

            AssetState meshState = assetManager->GetAssetState(object.mesh);

            if (meshState == AssetState::UNLOADED)
            {
                if (!assetManager->LoadAsset(object.mesh)) continue;

                meshState = assetManager->GetAssetState(object.mesh);
            }

            if (meshState != AssetState::LOADED) continue;

            MeshAsset* meshAsset = assetManager->GetAsset(object.mesh);
            if (!meshAsset) continue;

            VulkanMesh* mesh = GetOrCreateMesh(object.mesh.GetID(), meshAsset);
            if (!mesh) continue;

            VulkanMaterial* material = fallbackMaterial.get();

            if (object.material)
            {
                AssetState materialState = assetManager->GetAssetState(object.material);

                if (materialState == AssetState::UNLOADED)
                {
                    if (assetManager->LoadAsset(object.material))
                    {
                        materialState = assetManager->GetAssetState(object.material);
                    }
                }

                if (materialState == AssetState::LOADED)
                {
                    MaterialAsset* materialAsset = assetManager->GetAsset(object.material);

                    VulkanMaterial* loadedMaterial = GetOrCreateMaterial(object.material.GetID(), materialAsset, assetManager);

                    if (loadedMaterial) material = loadedMaterial;
                }
            }

            drawItems.push_back({ mesh, material, object.mvp, object.model });
        }
    }

    const VulkanFrameResult frameResult = renderer.DrawFrame(drawItems, sceneData, uiData);

    if (frameResult == VulkanFrameResult::RECREATE)
    {
        if (surfaceHost) RecreateRenderer(surfaceHost->GetSurfaceSize());
    }
    else if (frameResult == VulkanFrameResult::FAILED)
    {
        renderContext.Destroy();
    }
}

void VulkanGraphicsSystem::SetDebugView(GraphicsDebugView view)
{
    debugView = view;

    if (!renderContext.IsValid()) return;

    renderContext.GetRenderer().SetDebugView(view);
}

const GraphicsDeviceInfo& VulkanGraphicsSystem::GetDeviceInfo() const
{
    return device.GetInfo();
}

VulkanTexture* VulkanGraphicsSystem::GetOrCreateTexture(uint64 assetID, TextureAsset* textureAsset)
{
    if (!textureAsset) return nullptr;

    auto it = textures.find(assetID);
    if (it != textures.end()) return it->second.get();

    const std::vector<uint8>& pixels = textureAsset->GetPixels();

    if (pixels.empty() || textureAsset->GetWidth() == 0 || textureAsset->GetHeight() == 0) return nullptr;

    std::unique_ptr<VulkanTexture> texture = std::make_unique<VulkanTexture>();

    VulkanTextureCreateInfo textureInfo{};
    textureInfo.device = &device;
    textureInfo.pixels = pixels.data();
    textureInfo.width = textureAsset->GetWidth();
    textureInfo.height = textureAsset->GetHeight();
    textureInfo.format = textureAsset->GetFormat();
    textureInfo.config = config.texture;

    if (!texture->Create(textureInfo))
    {
        return nullptr;
    }

    VulkanTexture* result = texture.get();

    textures.emplace(assetID, std::move(texture));

    return result;
}

VulkanMaterial* VulkanGraphicsSystem::GetOrCreateMaterial(uint64 assetID, MaterialAsset* materialAsset, AssetManager* assetManager)
{
    if (!materialAsset || !assetManager) return nullptr;

    auto it = materials.find(assetID);
    if (it != materials.end()) return it->second.get();

    VulkanTexture* baseColorTexture = fallbackTexture.get();

    if (materialAsset->baseColorTexture)
    {
        AssetState textureState = assetManager->GetAssetState(materialAsset->baseColorTexture);

        if (textureState == AssetState::UNLOADED)
        {
            if (!assetManager->LoadAsset(materialAsset->baseColorTexture)) return nullptr;

            textureState = assetManager->GetAssetState(materialAsset->baseColorTexture);
        }

        if (textureState == AssetState::LOADED)
        {
            TextureAsset* textureAsset = assetManager->GetAsset(materialAsset->baseColorTexture);

            VulkanTexture* texture = GetOrCreateTexture(materialAsset->baseColorTexture.GetID(), textureAsset);

            if (texture) baseColorTexture = texture;
        }
    }

    VulkanMaterialCreateInfo materialInfo{};
    materialInfo.device = &device;
    materialInfo.descPool = materialDescPool;
    materialInfo.descSetLayout = materialDescSetLayout;
    materialInfo.baseColorTexture = baseColorTexture;
    materialInfo.uvScale = materialAsset->uvScale;
    materialInfo.baseColor = materialAsset->baseColor;
    materialInfo.roughness = materialAsset->roughness;
    materialInfo.metallic = materialAsset->metallic;
    materialInfo.ambientOcclusion = materialAsset->ambientOcclusion;

    std::unique_ptr<VulkanMaterial> material = std::make_unique<VulkanMaterial>();

    if (!material->Create(materialInfo)) return nullptr;

    VulkanMaterial* result = material.get();

    materials.emplace(assetID, std::move(material));

    return result;
}

VulkanMesh* VulkanGraphicsSystem::GetOrCreateMesh(uint64 assetID, MeshAsset* meshAsset)
{
    if (!meshAsset) return nullptr;

    auto it = meshes.find(assetID);
    if (it != meshes.end()) return it->second.get();

    const std::vector<MeshVertex>& vertices = meshAsset->GetVertices();
    const std::vector<uint32>& indices = meshAsset->GetIndices();

    if (vertices.empty() || indices.empty()) return nullptr;

    VulkanMeshCreateInfo meshInfo{};
    meshInfo.device = &device;
    meshInfo.vertexData = vertices.data();
    meshInfo.vertexDataSize = vertices.size() * sizeof(MeshVertex);
    meshInfo.indices = indices.data();
    meshInfo.indexCount = (uint32)indices.size();

    std::unique_ptr<VulkanMesh> mesh = std::make_unique<VulkanMesh>();

    if (!mesh->Create(meshInfo)) return nullptr;

    VulkanMesh* result = mesh.get();

    meshes.emplace(assetID, std::move(mesh));

    return result;
}
#endif
