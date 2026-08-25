// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Assets/AssetManager.h>
#include <MX/Core/Time/Clock.h>
#include <MX/EngineContext.h>
#include <MX/Graphics/AbstractGraphicsSystem.h>
#include <MX/Graphics/Graphics.h>
#include <MX/Graphics/GraphicsAccess.h>
#include <MX/Graphics/GraphicsSystemFactory.h>
#include <MX/Input/InputSystem.h>
#include <MX/Platform/WindowFactory.h>
#include <MX/Scene/SceneManager.h>
#include <MX/Scripting/ScriptSystem.h>
#include <MX/UI/UI.h>
#include <MX/UI/UIAccess.h>
#include <MX/Window/AbstractWindow.h>
#include <MX/Window/WindowEvent.h>
#include "GameApp.h"
#include <memory>
#include <optional>

struct GameApp::GameAppImpl
{
    std::unique_ptr<GameModule> gameModule;

    std::unique_ptr<AssetManager> assetManager;
    std::unique_ptr<AbstractGraphicsSystem> graphicsSystem;
    std::unique_ptr<AbstractWindow> gameWindow;
    std::unique_ptr<InputSystem> inputSystem;

    SceneManager sceneManager{};
    Graphics graphics{};
    UI ui{};

    std::optional<EngineContext> engineContext;

    Clock clock{};

    bool wantsExit = false;
    bool gameInitialized = false;

    explicit GameAppImpl(std::unique_ptr<GameModule> _gameModule) : gameModule(std::move(_gameModule)) {}

    bool Init()
    {
        if (!gameModule) return false;

        GameCreateInfo gameInfo{};
        gameModule->OnCreate(gameInfo);

        gameWindow = CreatePlatformWindow();

        if (!gameWindow) return false;
        if (!gameWindow->Create(gameInfo.window)) return false;

        gameWindow->Subscribe(&HandleWindowEventStatic, this);

        inputSystem = std::make_unique<InputSystem>();
        gameWindow->SetInputFeed(inputSystem.get());

        graphicsSystem = CreateGraphicsSystem();

        if (!graphicsSystem)
        {
            Destroy();
            return false;
        }

        if (!graphicsSystem->Create(*gameWindow))
        {
            Destroy();
            return false;
        }

        GraphicsAccess::Bind(graphics, graphicsSystem.get());

        assetManager = std::make_unique<AssetManager>();

        UIAccess::Bind(ui, assetManager.get());

        engineContext.emplace(
            *inputSystem, *gameWindow, *assetManager, sceneManager, graphics, ui,
            [this]() { wantsExit = true; }
        );

        gameModule->engineContext = &*engineContext;

        gameModule->OnInit();

        gameInitialized = true;

        clock.Reset();
        gameWindow->SetVisibility(true);

        return true;
    }

    void Destroy()
    {
        if (gameWindow) gameWindow->SetVisibility(false);

        if (gameModule)
        {
            if (gameInitialized) gameModule->OnExit();

            gameModule->engineContext = nullptr;
        }

        engineContext.reset();

        UIAccess::Bind(ui, nullptr);
        GraphicsAccess::Bind(graphics, nullptr);

        if (graphicsSystem) graphicsSystem->Destroy();

        assetManager.reset();

        if (gameWindow) gameWindow->SetInputFeed(nullptr);

        inputSystem.reset();

        if (gameWindow) gameWindow->Destroy();

        gameWindow.reset();
    }

    void Tick()
    {
        clock.Update();

        const float64 deltaTime = clock.Delta();

        inputSystem->BeginFrame();
        UIAccess::BeginFrame(ui);

        gameWindow->PollEvents();

        gameModule->OnUpdate(deltaTime);

        Scene* activeScene = sceneManager.GetActiveScene();

        if (activeScene)
        {
            ScriptSystem::Update(*activeScene, engineContext.value(), deltaTime);
        }

        graphicsSystem->RenderScene(activeScene, assetManager.get(), UIAccess::GetRenderData(ui));
    }

    void HandleWindowEvent(const WindowEvent& event)
    {
        switch (event.eventType)
        {
        case WindowEventType::RESIZED:
        {
            if (graphicsSystem)
            {
                graphicsSystem->RecreateRenderer(gameWindow->GetSurfaceSize());
            }

            break;
        }

        case WindowEventType::CLOSING:
        {
            wantsExit = true;

            break;
        }

        default:
            break;
        }
    }

    static void HandleWindowEventStatic(const WindowEvent& event, void* user)
    {
        static_cast<GameAppImpl*>(user)->HandleWindowEvent(event);
    }
};

GameApp::GameApp(std::unique_ptr<GameModule> gameModule)
    : _impl(std::make_unique<GameAppImpl>(std::move(gameModule))) {}

GameApp::~GameApp() = default;

void GameApp::Run()
{
    if (running) return;

    running = Init();
    if (!running) return;

    while (running)
    {
        Tick();

        if (exitRequested) running = false;
    }

    Destroy();

    running = false;
}

void GameApp::Exit()
{
    exitRequested = true;
}

bool GameApp::Init()
{
    return _impl->Init();
}

void GameApp::Destroy()
{
    _impl->Destroy();
}

void GameApp::Tick()
{
    _impl->Tick();

    if (_impl->wantsExit) Exit();
}
