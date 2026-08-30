// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "EditorApp.h"

#include <MX/Core/Time/Clock.h>
#include <MX/Platform/WindowFactory.h>
#include <MX/Window/AbstractWindow.h>
#include <MX/Window/WindowEvent.h>
#include <MX/Graphics/AbstractGraphicsSystem.h>
#include <MX/Graphics/GraphicsSystemFactory.h>
#include <MX/Graphics/Renderer/UI/UIRenderData.h>
#include <MX/Scene/Scene.h>
#include <MX/Input/InputSystem.h>
#include <MX/Graphics/Renderer/UI/AbstractUIRenderer.h>
#include <MX/Graphics/Renderer/UI/UITexture.h>
#include "UI/ImGui/ImGuiUIHost.h"
#include "UI/ImGui/ImGuiAdapter.h"

struct EditorApp::EditorAppImpl
{
    std::unique_ptr<AbstractWindow> editorWindow;
    std::unique_ptr<AbstractGraphicsSystem> graphicsSystem;
    std::unique_ptr<InputSystem> inputSystem;
    std::unique_ptr<AbstractUIHost> uiHost;

    Scene editorScene;

    Clock clock;

    bool wantsExit = false;

    bool Init()
    {
        editorWindow = CreatePlatformWindow();
        if (!editorWindow) return false;

        WindowConfig windowInfo{};
        windowInfo.title = "MagnetaX Editor";
        windowInfo.size = Size2i(1280, 720);
        windowInfo.resizable = true;

        if (!editorWindow->Create(windowInfo))
        {
            Destroy();
            return false;
        }

        editorWindow->Subscribe(&HandleWindowEventStatic, this);

        inputSystem = std::make_unique<InputSystem>();
        editorWindow->SetInputFeed(inputSystem.get());

        graphicsSystem = CreateGraphicsSystem();

        if (!graphicsSystem)
        {
            Destroy();
            return false;
        }

        if (!graphicsSystem->Create(*editorWindow))
        {
            Destroy();
            return false;
        }

        // For now it will be ImGUI only
        uiHost = std::make_unique<ImGuiUIHost>();

        UIHostCreateInfo uiHostInfo{};
        uiHostInfo.renderer = &graphicsSystem->GetUIRenderer();
        uiHostInfo.window = editorWindow.get();
        uiHostInfo.input = inputSystem.get();

        if (!uiHost->Create(uiHostInfo))
        {
            Destroy();
            return false;
        }

        clock.Reset();

        UIRenderData uiData{};
        graphicsSystem->RenderScene(nullptr, nullptr, uiData);

        editorWindow->SetVisibility(true);

        return true;
    }

    void Tick()
    {
        clock.Update();

        inputSystem->BeginFrame();
        editorWindow->PollEvents();

        uiHost->BeginFrame(clock.Delta());

        // Temp
        ImGui::Begin("Test");
        ImGui::TextUnformatted("TestText");
        ImGui::End();

        uiHost->EndFrame();

        UIRenderData uiData{};
        graphicsSystem->RenderScene(&editorScene, nullptr, uiData);
    }

    void Destroy()
    {
        if (graphicsSystem)
        {
            if (uiHost) uiHost->Destroy();
            graphicsSystem->Destroy();
        }

        uiHost.reset();
        graphicsSystem.reset();

        if (editorWindow)
        {
             editorWindow->SetVisibility(false);
             editorWindow->SetInputFeed(nullptr);
        }

        inputSystem.reset();

        if (editorWindow) editorWindow->Destroy();
        editorWindow.reset();
    }

    void HandleWindowEvent(const WindowEvent& event)
    {
        switch (event.eventType)
        {
        case WindowEventType::RESIZED:
        {
            if (graphicsSystem) graphicsSystem->RecreateRenderer(editorWindow->GetSurfaceSize());

            break;
        }
        case WindowEventType::CLOSING:
        {
            wantsExit = true;

            break;
        }
        case WindowEventType::FOCUS_GAINED:
        {
            if (uiHost) uiHost->SetFocus(true);

            break;
        }
        case WindowEventType::FOCUS_LOST:
        {
            if (uiHost) uiHost->SetFocus(false);

            break;
        }
        default:
            break;
        }
    }

    static void HandleWindowEventStatic(const WindowEvent& event, void* user)
    {
        static_cast<EditorAppImpl*>(user)->HandleWindowEvent(event);
    }
};

EditorApp::EditorApp() : _impl(std::make_unique<EditorAppImpl>()) {}

EditorApp::~EditorApp() = default;

void EditorApp::Run()
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

void EditorApp::Exit()
{
    exitRequested = true;
}

bool EditorApp::Init()
{
    return _impl->Init();
}

void EditorApp::Tick()
{
    _impl->Tick();

    if (_impl->wantsExit) Exit();
}

void EditorApp::Destroy()
{
    _impl->Destroy();
}
