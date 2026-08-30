// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Core/Math/Size.h>
#include <MX/Graphics/GraphicsConfig.h>
#include <MX/Graphics/GraphicsDebugView.h>
#include <MX/Graphics/GraphicsDeviceInfo.h>

class AssetManager;
class Scene;
class SurfaceHost;
struct UIRenderData;
class AbstractUIRenderer;

class AbstractGraphicsSystem
{
public:
    virtual ~AbstractGraphicsSystem() = default;

    virtual bool Create(const SurfaceHost& surfaceHost) = 0;
    virtual void Destroy() = 0;

    virtual bool RecreateRenderer(const Size2i& surfaceSize) = 0;

    virtual void RenderScene(Scene* scene, AssetManager* assetManager, const UIRenderData& uiData) = 0;

    virtual void SetDebugView(GraphicsDebugView view) = 0;

    const GraphicsConfig& GetConfig() const { return config; }

    virtual const GraphicsDeviceInfo& GetDeviceInfo() const = 0;

    virtual AbstractUIRenderer& GetUIRenderer() = 0;

protected:
    GraphicsConfig config{};
};
