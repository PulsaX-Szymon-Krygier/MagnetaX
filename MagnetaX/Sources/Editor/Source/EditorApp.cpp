// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "EditorApp.h"

#include <MX/Platform/WindowFactory.h>
#include <MX/Window/AbstractWindow.h>
#include <MX/Window/WindowEvent.h>
#include <MX/Graphics/AbstractGraphicsSystem.h>
#include <MX/Graphics/GraphicsSystemFactory.h>
#include <MX/Graphics/Renderer/UI/UIRenderData.h>
#include <memory>

struct EditorApp::EditorAppImpl
{
    std::unique_ptr<AbstractWindow> editorWindow;
    std::unique_ptr<AbstractGraphicsSystem> graphicsSystem;

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

        UIRenderData uiData{};
        graphicsSystem->RenderScene(nullptr, nullptr, uiData);

        editorWindow->SetVisibility(true);

        return true;
    }

    void Tick()
    {
        editorWindow->PollEvents();

        UIRenderData uiData{};
        graphicsSystem->RenderScene(nullptr, nullptr, uiData);
    }

    void Destroy()
    {
        if (editorWindow) editorWindow->SetVisibility(false);

        if (graphicsSystem) graphicsSystem->Destroy();
        graphicsSystem.reset();

        if (editorWindow) editorWindow->Destroy();
        editorWindow.reset();
    }

    void HandleWindowEvent(const WindowEvent& event)
    {
        switch (event.eventType)
        {
        case WindowEventType::RESIZED:
        {
            if (graphicsSystem)
            {
                graphicsSystem->RecreateRenderer(editorWindow->GetSurfaceSize());
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
