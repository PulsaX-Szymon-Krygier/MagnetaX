// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Graphics/Graphics.h>
#include <MX/Scene/Entity.h>
#include <MX/UI/UI.h>

class AssetManager;
class EngineContext;
class Input;
class IWindow;
class Scene;
class SceneManager;
class ScriptSystem;

class Script
{
public:
    virtual ~Script() = default;

protected:
    virtual void OnStart() {}
    virtual void OnUpdate(float64 deltaTime) { (void)deltaTime; }

    Entity GetEntity() const { return entity; }

    AssetManager& GetAssetManager() const;
    Graphics& GetGraphics() const;
    Input& GetInput() const;
    IWindow& GetWindow() const;
    SceneManager& GetSceneManager() const;
    UI& GetUI() const;

    Scene* GetActiveScene() const;
    void SetActiveScene(Scene& scene);

    void RequestExit() const;

private:
    friend class ScriptSystem;

    Entity entity{};
    EngineContext* engineContext = nullptr;

    bool started = false;
};
