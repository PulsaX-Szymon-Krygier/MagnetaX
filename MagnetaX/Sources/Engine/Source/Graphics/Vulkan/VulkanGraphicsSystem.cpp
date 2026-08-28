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
#include <Graphics/Vulkan/Renderer/VulkanCommandPool.h>
#include "VulkanInitializers.h"
#include <vector>

namespace
{
    constexpr uint32 SPECULAR_ENV_SIZE = 256;
    constexpr uint32 SPECULAR_ENV_MIP_LEVELS = 5;

    constexpr uint32 BRDF_LUT_SIZE = 512;
}

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

    VulkanEquirectPassCreateInfo equirectInfo{};
    equirectInfo.device = &device;
    equirectInfo.outFormat = VulkanImageFormat::FromImageFormat(ImageFormat::RGBA16_FLOAT);

    if (!equirectPass.Create(equirectInfo))
    {
        Destroy();
        return false;
    }

    VulkanSpecularEnvPassCreateInfo specularEnvInfo{};
    specularEnvInfo.device = &device;
    specularEnvInfo.outFormat = VulkanImageFormat::FromImageFormat(ImageFormat::RGBA16_FLOAT);

    if (!specularEnvPass.Create(specularEnvInfo))
    {
        Destroy();
        return false;
    }

    VulkanBRDFLUTPassCreateInfo brdfLUTInfo{};
    brdfLUTInfo.device = &device;
    brdfLUTInfo.outFormat = VulkanImageFormat::FromImageFormat(ImageFormat::RGBA16_FLOAT);

    if (!brdfLUTPass.Create(brdfLUTInfo))
    {
        Destroy();
        return false;
    }

    if (!GenerateBRDFLUT())
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

    equirectPass.Destroy();
    specularEnvPass.Destroy();
    brdfLUTPass.Destroy();

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

    // Temporary place!!!
    DestroyEnvironmentCubemap();
    DestroySpecularEnvironment();
    DestroyBRDFLUT();
    if (environmentTexture) environmentTexture->Destroy();
    environmentTexture.reset();
    environmentMapAssetID = 0;
    // -----

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

    // --------------------------------------------------------------
    // Temporary place for env texture/map code
    // I will move it to proper place after testing
    const uint64 envAssetID = sceneData.environmentMap.GetID();

    if (envAssetID != environmentMapAssetID)
    {
        if (device.GetDevice())
        {
            vkDeviceWaitIdle(device.GetDevice());
        }

        DestroyEnvironmentCubemap();
        DestroySpecularEnvironment();
        //DestroyBRDFLUT();

        if (environmentTexture) environmentTexture->Destroy();
        environmentTexture.reset();

        if (sceneData.environmentMap && assetManager)
        {
            AssetState envState = assetManager->GetAssetState(sceneData.environmentMap);

            if (envState == AssetState::UNLOADED)
            {
                if (assetManager->LoadAsset(sceneData.environmentMap))
                {
                    envState = assetManager->GetAssetState(sceneData.environmentMap);
                }
            }

            if (envState == AssetState::LOADED)
            {
                TextureAsset* envTexture = assetManager->GetAsset(sceneData.environmentMap);

                if (envTexture && envTexture->GetFormat() == ImageFormat::RGBA32_FLOAT)
                {
                    VulkanTextureCreateInfo textureInfo{};
                    textureInfo.pixels = envTexture->GetPixels().data();
                    textureInfo.width = envTexture->GetWidth();
                    textureInfo.height = envTexture->GetHeight();
                    textureInfo.config.mipmaps = false;
                    textureInfo.config.anisotropy = 1.0f;
                    textureInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                    textureInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                    textureInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                    textureInfo.device = &device;
                    //textureInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                    textureInfo.format = envTexture->GetFormat();

                    environmentTexture = std::make_unique<VulkanTexture>();

                    bool environmentReady = environmentTexture->Create(textureInfo);

                    const uint32 width = envTexture->GetWidth();
                    const uint32 height = envTexture->GetHeight();

                    if (environmentReady && (height == 0 || width != height * 2))
                    {
                        environmentReady = false;
                    }

                    const uint32 faceSize = width / 4;

                    uint32 environmentMipLevels = 1;

                    for (uint32 size = faceSize; size > 1; size >>= 1)
                    {
                        ++environmentMipLevels;
                    }

                    if (environmentReady)
                    {
                        VulkanImageCreateInfo imageInfo{};
                        imageInfo.device = &device;
                        imageInfo.extent = { faceSize, faceSize };
                        imageInfo.format = ImageFormat::RGBA16_FLOAT;
                        //imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                        imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                        imageInfo.mipLevels = environmentMipLevels;
                        imageInfo.arrayLayers = 6;
                        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
                        imageInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;

                        if (!environmentCubemap.Create(imageInfo))
                        {
                            environmentReady = false;
                        }
                    }

                    if (environmentReady)
                    {
                        for (uint32 i = 0; i < environmentCubemapFaceViews.size(); ++i)
                        {
                            VkImageViewCreateInfo viewInfo{};
                            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                            viewInfo.image = environmentCubemap.GetImage();
                            viewInfo.format = environmentCubemap.GetFormat();
                            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                            viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                            viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                            viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                            viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
                            viewInfo.subresourceRange.baseMipLevel = 0;
                            viewInfo.subresourceRange.baseArrayLayer = i;
                            viewInfo.subresourceRange.layerCount = 1;
                            viewInfo.subresourceRange.levelCount = 1;
                            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                            viewInfo.pNext = nullptr;

                            if (vkCreateImageView(device.GetDevice(), &viewInfo, nullptr, &environmentCubemapFaceViews[i]) != VK_SUCCESS)
                            {
                                environmentReady = false;
                                break;
                            }
                        }
                    }

                    if (environmentReady)
                    {
                        VkSamplerCreateInfo samplerInfo{};
                        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                        samplerInfo.magFilter = VK_FILTER_LINEAR;
                        samplerInfo.minFilter = VK_FILTER_LINEAR;
                        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                        samplerInfo.minLod = 0.0f;
                        //samplerInfo.maxLod = 0.0f;
                        samplerInfo.maxLod = (float32)(environmentMipLevels - 1);

                        if (vkCreateSampler(device.GetDevice(), &samplerInfo, nullptr, &environmentCubemapSampler) != VK_SUCCESS)
                        {
                            environmentReady = false;
                        }
                    }

                    if (environmentReady)
                    {
                        VulkanImageCreateInfo imageInfo{};
                        imageInfo.device = &device;
                        imageInfo.extent = { SPECULAR_ENV_SIZE , SPECULAR_ENV_SIZE };
                        imageInfo.format = ImageFormat::RGBA16_FLOAT;
                        imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                        imageInfo.mipLevels = SPECULAR_ENV_MIP_LEVELS;
                        imageInfo.arrayLayers = 6;
                        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
                        imageInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;

                        if (!specularEnvironmentCubemap.Create(imageInfo))
                        {
                            environmentReady = false;
                        }
                    }

                    if (environmentReady)
                    {
                        specularEnvironmentViews.resize(SPECULAR_ENV_MIP_LEVELS * 6);

                        for (uint32 mipLevel = 0; mipLevel < SPECULAR_ENV_MIP_LEVELS; ++mipLevel)
                        {
                            for (uint32 faceIndex = 0; faceIndex < 6; ++faceIndex)
                            {
                                const uint32 viewIndex = mipLevel * 6 + faceIndex;

                                VkImageViewCreateInfo viewInfo{};
                                viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                                viewInfo.image = specularEnvironmentCubemap.GetImage();
                                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                                viewInfo.format = specularEnvironmentCubemap.GetFormat();
                                viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                                viewInfo.subresourceRange.baseMipLevel = mipLevel;
                                viewInfo.subresourceRange.levelCount = 1;
                                viewInfo.subresourceRange.baseArrayLayer = faceIndex;
                                viewInfo.subresourceRange.layerCount = 1;

                                if (vkCreateImageView(device.GetDevice(), &viewInfo, nullptr, &specularEnvironmentViews[viewIndex]) != VK_SUCCESS)
                                {
                                    environmentReady = false;
                                    break;
                                }
                            }

                            if (!environmentReady) break;
                        }
                    }

                    if (environmentReady)
                    {
                        VkSamplerCreateInfo samplerInfo{};
                        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                        samplerInfo.magFilter = VK_FILTER_LINEAR;
                        samplerInfo.minFilter = VK_FILTER_LINEAR;
                        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                        samplerInfo.minLod = 0.0f;
                        samplerInfo.maxLod = (float32)(SPECULAR_ENV_MIP_LEVELS - 1);

                        if (vkCreateSampler(device.GetDevice(), &samplerInfo, nullptr, &specularEnvironmentSampler) != VK_SUCCESS)
                        {
                            environmentReady = false;
                        }
                    }

                    if (environmentReady)
                    {
                        const VkExtent2D environmentExtent{ faceSize, faceSize };

                        if (!ConvertEnvironmentCubemap(environmentExtent))
                        {
                            environmentReady = false;
                        }
                    }

                    if (!environmentReady)
                    {
                        DestroyEnvironmentCubemap();
                        DestroySpecularEnvironment();

                        if (environmentTexture) environmentTexture->Destroy();
                        environmentTexture.reset();
                    }
                }
            }
        }

        // Rn it's ok for testing but it sets ID no matter if asset exists
        // Add proper logic in the future
        environmentMapAssetID = envAssetID;
    }
    // --------------------------------------------------------------

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

    const VulkanFrameResult frameResult = renderer.DrawFrame(drawItems, sceneData, uiData, environmentCubemap.GetImageView(),
        environmentCubemapSampler, specularEnvironmentCubemap.GetImageView(), specularEnvironmentSampler,
        brdfLUT.GetImageView(), brdfLUTSampler);

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

void VulkanGraphicsSystem::DestroyEnvironmentCubemap()
{
    const VkDevice buffDevice = device.GetDevice();

    if (buffDevice)
    {
        if (environmentCubemapSampler) vkDestroySampler(buffDevice, environmentCubemapSampler, nullptr);
        environmentCubemapSampler = VK_NULL_HANDLE;

        for (VkImageView& view : environmentCubemapFaceViews)
        {
            if (view) vkDestroyImageView(buffDevice, view, nullptr);
            view = VK_NULL_HANDLE;
        }
    }

    environmentCubemap.Destroy();
}

bool VulkanGraphicsSystem::ConvertEnvironmentCubemap(VkExtent2D extent)
{
    if (!environmentTexture || !environmentCubemap.GetImage() || !environmentCubemapSampler) return false;
    if (!specularEnvironmentCubemap.GetImage() || specularEnvironmentViews.size() != SPECULAR_ENV_MIP_LEVELS * 6) return false;
    if (extent.width == 0 || extent.height == 0) return false;

    const VkDevice buffDevice = device.GetDevice();
    if (!buffDevice) return false;

    VulkanCommandPool commandPool;

    if (!commandPool.Create(buffDevice, device.GetGraphicsQueueFamily())) return false;

    const VkCommandBuffer cmdBuffer = commandPool.AllocateCommandBuffer();

    if (!cmdBuffer)
    {
        commandPool.Destroy();
        return false;
    }

    const VkCommandBufferBeginInfo beginInfo = VulkanInitializers::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS)
    {
        commandPool.Destroy();
        return false;
    }

    VulkanEquirectPassRenderInfo renderInfo{};
    renderInfo.cmdBuffer = cmdBuffer;
    renderInfo.sourceTexture = environmentTexture.get();
    renderInfo.targetImage = environmentCubemap.GetImage();
    renderInfo.targetViews = std::span<const VkImageView>(environmentCubemapFaceViews);
    renderInfo.extent = extent;

    equirectPass.Record(renderInfo);

    uint32 environmentMipLevels = 1;

    for (uint32 size = extent.width; size > 1; size >>= 1)
    {
        ++environmentMipLevels;
    }

    int32 mipSize = (int32)extent.width;

    for (uint32 mipLevel = 1; mipLevel < environmentMipLevels; ++mipLevel)
    {
        VkImageMemoryBarrier2 mipBarriers[2]{};

        mipBarriers[0] = VulkanInitializers::ImageMemoryBarrier(
            environmentCubemap.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
            mipLevel - 1, 1, 0, 6
        );

        mipBarriers[1] = VulkanInitializers::ImageMemoryBarrier(
            environmentCubemap.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
            mipLevel, 1, 0, 6
        );

        VkDependencyInfo mipDependency{};
        mipDependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        mipDependency.imageMemoryBarrierCount = 2;
        mipDependency.pImageMemoryBarriers = mipBarriers;

        vkCmdPipelineBarrier2(cmdBuffer, &mipDependency);

        const int32 nextMipSize = mipSize > 1 ? mipSize / 2 : 1;

        VkImageBlit blit{};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = mipLevel - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 6;
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipSize, mipSize, 1 };

        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = mipLevel;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 6;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { nextMipSize, nextMipSize, 1 };

        vkCmdBlitImage(
            cmdBuffer,
            environmentCubemap.GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            environmentCubemap.GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit, VK_FILTER_LINEAR
        );

        mipBarriers[0] = VulkanInitializers::ImageMemoryBarrier(
            environmentCubemap.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
            mipLevel - 1, 1, 0, 6
        );

        mipBarriers[1] = VulkanInitializers::ImageMemoryBarrier(
            environmentCubemap.GetImage(), VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
            mipLevel, 1, 0, 6
        );

        vkCmdPipelineBarrier2(cmdBuffer, &mipDependency);

        mipSize = nextMipSize;
    }

    VulkanSpecularEnvPassRenderInfo specularInfo{};
    specularInfo.cmdBuffer = cmdBuffer;
    specularInfo.sourceView = environmentCubemap.GetImageView();
    specularInfo.sourceSampler = environmentCubemapSampler;
    specularInfo.targetImage = specularEnvironmentCubemap.GetImage();
    specularInfo.targetViews = std::span<const VkImageView>(specularEnvironmentViews);
    specularInfo.extent = { SPECULAR_ENV_SIZE, SPECULAR_ENV_SIZE };
    specularInfo.mipLevels = SPECULAR_ENV_MIP_LEVELS;

    specularEnvPass.Record(specularInfo);

    if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS)
    {
        commandPool.Destroy();
        return false;
    }

    VkCommandBufferSubmitInfo cmdBufferInfo{};
    cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    cmdBufferInfo.commandBuffer = cmdBuffer;
    cmdBufferInfo.pNext = nullptr;

    VkSubmitInfo2 submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &cmdBufferInfo;
    submitInfo.pNext = nullptr;

    const VkQueue graphicsQueue = device.GetGraphicsQueue();

    if (vkQueueSubmit2(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        commandPool.Destroy();
        return false;
    }

    if (vkQueueWaitIdle(graphicsQueue) != VK_SUCCESS)
    {
        commandPool.Destroy();
        return false;
    }

    commandPool.Destroy();

    return true;
}

void VulkanGraphicsSystem::DestroySpecularEnvironment()
{
    const VkDevice buffDevice = device.GetDevice();

    if (buffDevice)
    {
        if (specularEnvironmentSampler) vkDestroySampler(buffDevice, specularEnvironmentSampler, nullptr);
        specularEnvironmentSampler = VK_NULL_HANDLE;

        for (VkImageView& view : specularEnvironmentViews)
        {
            if (view) vkDestroyImageView(buffDevice, view, nullptr);
        }
    }

    specularEnvironmentViews.clear();
    specularEnvironmentCubemap.Destroy();
}

bool VulkanGraphicsSystem::GenerateBRDFLUT()
{
    const VkDevice buffDevice = device.GetDevice();
    if (!buffDevice) return false;

    DestroyBRDFLUT();

    VulkanImageCreateInfo imageInfo{};
    imageInfo.device = &device;
    imageInfo.extent = { BRDF_LUT_SIZE, BRDF_LUT_SIZE };
    imageInfo.format = ImageFormat::RGBA16_FLOAT;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    if (!brdfLUT.Create(imageInfo))
    {
        DestroyBRDFLUT();
        return false;
    }

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(buffDevice, &samplerInfo, nullptr, &brdfLUTSampler) != VK_SUCCESS)
    {
        DestroyBRDFLUT();
        return false;
    }

    VulkanCommandPool commandPool;

    if (!commandPool.Create(buffDevice, device.GetGraphicsQueueFamily()))
    {
        DestroyBRDFLUT();
        return false;
    }

    const VkCommandBuffer cmdBuffer = commandPool.AllocateCommandBuffer();

    if (!cmdBuffer)
    {
        commandPool.Destroy();
        DestroyBRDFLUT();
        return false;
    }

    const VkCommandBufferBeginInfo beginInfo = VulkanInitializers::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS)
    {
        commandPool.Destroy();
        DestroyBRDFLUT();
        return false;
    }

    VulkanBRDFLUTPassRenderInfo renderInfo{};
    renderInfo.cmdBuffer = cmdBuffer;
    renderInfo.targetImage = brdfLUT.GetImage();
    renderInfo.targetView = brdfLUT.GetImageView();
    renderInfo.extent = { BRDF_LUT_SIZE, BRDF_LUT_SIZE };

    brdfLUTPass.Record(renderInfo);

    if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS)
    {
        commandPool.Destroy();
        DestroyBRDFLUT();
        return false;
    }

    VkCommandBufferSubmitInfo cmdBufferInfo{};
    cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    cmdBufferInfo.commandBuffer = cmdBuffer;

    VkSubmitInfo2 submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &cmdBufferInfo;

    const VkQueue graphicsQueue = device.GetGraphicsQueue();

    if (vkQueueSubmit2(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        commandPool.Destroy();
        DestroyBRDFLUT();
        return false;
    }

    if (vkQueueWaitIdle(graphicsQueue) != VK_SUCCESS)
    {
        commandPool.Destroy();
        DestroyBRDFLUT();
        return false;
    }

    commandPool.Destroy();

    return true;
}

void VulkanGraphicsSystem::DestroyBRDFLUT()
{
    const VkDevice buffDevice = device.GetDevice();

    if (buffDevice && brdfLUTSampler)
    {
        vkDestroySampler(buffDevice, brdfLUTSampler, nullptr);
    }

    brdfLUTSampler = VK_NULL_HANDLE;
    brdfLUT.Destroy();
}
#endif
