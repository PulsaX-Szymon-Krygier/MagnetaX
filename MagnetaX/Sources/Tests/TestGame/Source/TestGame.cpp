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
    createInfo.window.title = "MagnetaX TestGame FPS: ?";
    createInfo.window.size = Size2i(1280, 720);
}

void TestGame::OnInit()
{
    GetSceneManager().SetActiveScene(scene1);

    SceneEnvironment& env = scene1.GetEnvironment();
    env.backgroundColor = Vector3f(0.1f, 0.4f, 0.6f);
    env.ambientLightColor = Vector3f(1.0f, 1.0f, 1.0f);
    env.ambientLightIntensity = 0.01f;

    Entity testEntity = scene1.CreateEntity();
    CameraComponent& camera = testEntity.AddComponent<CameraComponent>();
    camera.exposureEV = -3.0f;

    testEntity.AddComponent<ScriptComponent>().AddScript<CameraScript>();

    scene1.SetActiveCamera(testEntity);

    // Assets
    std::filesystem::path assetsPath = IOUtils::GetExecutableDirectory() / "Assets";

    AssetHandle<TextureAsset> txtOrange = GetAssetManager().CreateAsset<TextureAsset>(AssetSource((assetsPath / "Textures/orange.png").string()));
    AssetHandle<TextureAsset> txtGrey = GetAssetManager().CreateAsset<TextureAsset>(AssetSource((assetsPath / "Textures/grey.png").string()));

    AssetHandle<MeshAsset> meshCube = GetAssetManager().CreateAsset<MeshAsset>(AssetSource((assetsPath / "Models/cube.obj").string()));
    AssetHandle<MeshAsset> meshSphere = GetAssetManager().CreateAsset<MeshAsset>(AssetSource((assetsPath / "Models/sphere.obj").string()), true);

    AssetHandle<TextureAsset> hdri1 = GetAssetManager().CreateAsset<TextureAsset>(AssetSource((assetsPath / "HDRI/qwantani_moonrise_puresky_4k.hdr").string()), ImageFormat::RGBA32_FLOAT);
    AssetHandle<TextureAsset> hdri2 = GetAssetManager().CreateAsset<TextureAsset>(AssetSource((assetsPath / "HDRI/malagas_ferry_road_4k.hdr").string()), ImageFormat::RGBA32_FLOAT);

    env.environmentMap = hdri1;

    auto createMaterial = 
        [&](const Vector4f& color, float32 roughness, float32 metallic, AssetHandle<TextureAsset> texture = {}, float32 scale = 1.0f)
        {
            AssetHandle<MaterialAsset> buff = GetAssetManager().CreateAsset<MaterialAsset>();
            MaterialAsset* mat = GetAssetManager().GetAsset(buff);

            mat->baseColor = color;
            mat->roughness = roughness;
            mat->metallic = metallic;
            mat->baseColorTexture = texture;
            mat->uvScale = Vector2f(scale);

            return buff;
        };

    AssetHandle<MaterialAsset> matWall = createMaterial(Vector4f(0.5f, 0.6f, 0.7f, 1.0f), 0.9f, 0.0f);
    AssetHandle<MaterialAsset> matPlatform = createMaterial(Vector4f(0.8f, 0.8f, 0.8f, 1.0f), 1.0f, 0.0f, txtGrey, 4.0f);
    AssetHandle<MaterialAsset> mat1 = createMaterial(Vector4f(1.0f), 0.0f, 1.0f);
    AssetHandle<MaterialAsset> mat2 = createMaterial(Vector4f(1.0f), 0.2f, 1.0f);
    AssetHandle<MaterialAsset> mat3 = createMaterial(Vector4f(1.0f), 0.0f, 0.0f);
    AssetHandle<MaterialAsset> mat4 = createMaterial(Vector4f(1.0f), 0.8f, 0.0f);

    // Entities
    auto createEntity =
        [&](AssetHandle<MeshAsset> mesh, AssetHandle<MaterialAsset> material, const Vector3f& pos, const Vector3f& scale)
        {
            Entity entity = scene1.CreateEntity();
            
            TransformComponent* transform = entity.GetComponent<TransformComponent>();
            transform->position = pos;
            transform->scale = scale;

            entity.AddComponent<MeshComponent>(mesh);
            entity.AddComponent<MaterialComponent>(material);

            return entity;
        };

    createEntity(meshCube, matPlatform, Vector3f(0.0f, -0.5f, -2.0f), Vector3f(12.0f, 1.0f, 12.0f));

    createEntity(meshSphere, mat1, Vector3f(-4.5f, 1.0f, -2.0f), Vector3f(2.0f));
    createEntity(meshSphere, mat2, Vector3f(-1.5f, 1.0f, -2.0f), Vector3f(2.0f));
    createEntity(meshSphere, mat3, Vector3f(1.5f, 1.0f, -2.0f), Vector3f(2.0f));
    createEntity(meshSphere, mat4, Vector3f(4.5f, 1.0f, -2.0f), Vector3f(2.0f));

    // Lights
    auto createLight =
        [&](LightType type, const Vector3f& pos, const Vector3f& color, float32 intensity, float32 range)
        {
            Entity entity = scene1.CreateEntity();
            entity.GetComponent<TransformComponent>()->position = pos;

            LightComponent& light = entity.AddComponent<LightComponent>();
            light.type = type;
            light.color = color;
            light.intensity = intensity;
            light.range = range;

            return entity;
        };

    createLight(LightType::POINT, Vector3f(-4.0f, 4.0f, 1.0f), Vector3f(1.0f, 0.3f, 0.1f), 30.0f, 10.0f);
    createLight(LightType::POINT, Vector3f(4.0f, 4.0f, 1.0f), Vector3f(0.8f, 0.9f, 1.0f), 25.0f, 10.0f);

    Entity sun = createLight(LightType::DIRECTIONAL, Vector3f(0.0f), Vector3f(1.0f, 1.0f, 1.0f), 10.0f, 0.0f);
    sun.GetComponent<TransformComponent>()->rotation = Quaternion::FromYawPitchRollDegrees(-30.0f, -30.0f, 0.0f);
    LightComponent* dirLight = sun.GetComponent<LightComponent>();
    dirLight->intensity = 10.0f;
    dirLight->castShadows = true;

    Entity spot = createLight(LightType::SPOT, Vector3f(0.0f, 5.0f, -2.0f), Vector3f(1.0f, 0.9f, 0.8f), 35.0f, 12.0f);
    spot.GetComponent<TransformComponent>()->rotation = Quaternion::FromYawPitchRollDegrees(0.0f, -90.0f, 0.0f);

    LightComponent* spotLight = spot.GetComponent<LightComponent>();
    spotLight->innerConeAngle = 25.0f;
    spotLight->outerConeAngle = 40.0f;
    spotLight->castShadows = true;
    spotLight->intensity = 0.0f;
}

void TestGame::OnUpdate(float64 deltaTime)
{
    fpsTimer += deltaTime;
    ++fpsFrameCount;

    if (fpsTimer >= 0.5)
    {
        const float64 fps = fpsFrameCount / fpsTimer;

        std::ostringstream title;
        title << "MagnetaX TestGame FPS: " << std::fixed << std::setprecision(0) << fps;

        GetWindow().SetTitle(title.str());

        fpsTimer = 0.0;
        fpsFrameCount = 0;
    }

    const Input::Keyboard keyboard = GetInput().GetKeyboard();

    if (keyboard.KeyPressed(KeyboardKeys::ESCAPE)) RequestExit();
    if (keyboard.KeyPressed(KeyboardKeys::F1)) GetGraphics().SetDebugView(GraphicsDebugView::FINAL);
    if (keyboard.KeyPressed(KeyboardKeys::F2)) GetGraphics().SetDebugView(GraphicsDebugView::ALBEDO);
    if (keyboard.KeyPressed(KeyboardKeys::F3)) GetGraphics().SetDebugView(GraphicsDebugView::NORMAL);
    if (keyboard.KeyPressed(KeyboardKeys::F4)) GetGraphics().SetDebugView(GraphicsDebugView::MATERIAL);
    if (keyboard.KeyPressed(KeyboardKeys::F5)) GetGraphics().SetDebugView(GraphicsDebugView::DEPTH);
}

void TestGame::OnExit()
{}

MX_GAME_REGISTER(TestGame)
