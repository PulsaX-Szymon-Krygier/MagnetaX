// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Scripting/Script.h>
#include <MX/EngineContext.h>
#include <MX/Scene/SceneManager.h>

AssetManager& Script::GetAssetManager() const
{
    return engineContext->GetAssetManager();
}

Graphics& Script::GetGraphics() const
{
    return engineContext->GetGraphics();
}

Input& Script::GetInput() const
{
    return engineContext->GetInput();
}

IWindow& Script::GetWindow() const
{
    return engineContext->GetWindow();
}

SceneManager& Script::GetSceneManager() const
{
    return engineContext->GetSceneManager();
}

UI& Script::GetUI() const
{
    return engineContext->GetUI();
}

Scene* Script::GetActiveScene() const
{
    return engineContext->GetSceneManager().GetActiveScene();
}

void Script::SetActiveScene(Scene& scene)
{
    engineContext->GetSceneManager().SetActiveScene(scene);
}

void Script::RequestExit() const
{
    engineContext->RequestExit();
}
