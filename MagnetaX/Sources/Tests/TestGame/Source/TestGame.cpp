// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Assets/Font/FontAsset.h>
#include <MX/Assets/Material/MaterialAsset.h>
#include <MX/Assets/Mesh/MeshAsset.h>
#include <MX/Assets/Texture/TextureAsset.h>
#include <MX/Core/IO/IOUtils.h>
#include <MX/Game/GameEntryPoint.h>
#include "TestGame.h"
#include "CameraScript.h"
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

TestGame::TestGame() = default;
TestGame::~TestGame() = default;

void TestGame::OnCreate(GameCreateInfo& createInfo)
{
    createInfo.window.title = "MagnetaX TestGame";
    createInfo.window.size = Size2i(1280, 720);
}

void TestGame::OnInit()
{
    GetSceneManager().SetActiveScene(scene1);

    SceneEnvironment& env = scene1.GetEnvironment();
    env.backgroundColor = Vector3f(0.1f, 0.4f, 0.6f);
    env.ambientLightColor = Vector3f(0.6f, 0.7f, 1.0f);
    env.ambientLightIntensity = 0.1f;

    Entity testEntity = scene1.CreateEntity();
    testEntity.AddComponent<CameraComponent>();

    scene1.SetActiveCamera(testEntity);

    // Remember: I removed content for now before public code release
    // too much deps rn, obj textures etc. 
}

void TestGame::OnUpdate(float64 deltaTime)
{
    const Input::Keyboard keyboard = GetInput().GetKeyboard();

    if (keyboard.KeyPressed(KeyboardKeys::ESCAPE)) RequestExit();
}

void TestGame::OnExit()
{}

MX_GAME_REGISTER(TestGame)
