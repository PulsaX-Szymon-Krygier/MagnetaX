// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Game/GameModule.h>
#include <MX/EngineContext.h>

AssetManager& GameModule::GetAssetManager() const
{
    return engineContext->GetAssetManager();
}

Graphics& GameModule::GetGraphics() const
{
    return engineContext->GetGraphics();
}

Input& GameModule::GetInput() const
{
    return engineContext->GetInput();
}

IWindow& GameModule::GetWindow() const
{
    return engineContext->GetWindow();
}

SceneManager& GameModule::GetSceneManager() const
{
    return engineContext->GetSceneManager();
}

UI& GameModule::GetUI() const
{
    return engineContext->GetUI();
}

Scene* GameModule::GetActiveScene() const
{
    return engineContext->GetSceneManager().GetActiveScene();
}

void GameModule::SetActiveScene(Scene& scene)
{
    engineContext->GetSceneManager().SetActiveScene(scene);
}

void GameModule::RequestExit() const
{
    engineContext->RequestExit();
}
