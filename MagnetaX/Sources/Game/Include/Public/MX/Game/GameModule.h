// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Assets/Assets.h>
#include <MX/Graphics/Graphics.h>
#include <MX/Input/Input.h>
#include <MX/Scene/ECS.h>
#include <MX/Scene/SceneManager.h>
#include <MX/UI/UI.h>
#include <MX/Window/IWindow.h>
#include "GameCreateInfo.h"

class EngineContext;

class GameModule
{
public:
    virtual ~GameModule() = default;

protected:
    virtual void OnCreate(GameCreateInfo& createInfo) { (void)createInfo; }
    virtual void OnInit() {}
    virtual void OnUpdate(float64 deltaTime) { (void)deltaTime; }
    virtual void OnExit() {}

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
    friend class GameApp;

    EngineContext* engineContext = nullptr;
};
