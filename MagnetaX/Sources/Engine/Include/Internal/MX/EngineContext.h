// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <utility>
#include <functional>

class AssetManager;
class Graphics;
class Input;
class IWindow;
class SceneManager;
class UI;

class EngineContext
{
public:
    EngineContext(Input& input, IWindow& window, AssetManager& assetManager, SceneManager& sceneManager, Graphics& graphics, UI& ui,
        std::function<void()> requestExit)
        : input(&input), window(&window), assetManager(&assetManager), sceneManager(&sceneManager), graphics(&graphics), ui(&ui),
        requestExit(std::move(requestExit)) {}

    Input& GetInput() const { return *input; }
    IWindow& GetWindow() const { return *window; }
    AssetManager& GetAssetManager() const { return *assetManager; }
    SceneManager& GetSceneManager() const { return *sceneManager; }
    Graphics& GetGraphics() const { return *graphics; }
    UI& GetUI() const { return *ui; }

    void RequestExit() const { if (requestExit) requestExit(); }

private:
    Input* input = nullptr;
    IWindow* window = nullptr;
    AssetManager* assetManager = nullptr;
    SceneManager* sceneManager = nullptr;
    Graphics* graphics = nullptr;
    UI* ui = nullptr;

    std::function<void()> requestExit;
};
