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
#include <imgui.h>
#include <memory>

struct EditorApp::EditorAppImpl
{
    std::unique_ptr<AbstractWindow> editorWindow;
    std::unique_ptr<AbstractGraphicsSystem> graphicsSystem;
    std::unique_ptr<InputSystem> inputSystem;

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

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        clock.Reset();

        UIRenderData uiData{};
        graphicsSystem->RenderScene(nullptr, nullptr, uiData);

        editorWindow->SetVisibility(true);

        return true;
    }

    void Tick()
    {
        inputSystem->BeginFrame();
        editorWindow->PollEvents();

        BeginImGuiFrame();

        //ImGui::EndFrame();

        UIRenderData uiData{};
        graphicsSystem->RenderScene(&editorScene, nullptr, uiData);
    }

    void Destroy()
    {
        if (graphicsSystem) graphicsSystem->Destroy();
        graphicsSystem.reset();

        if (editorWindow)
        {
             editorWindow->SetVisibility(false);
             editorWindow->SetInputFeed(nullptr);
        }

        inputSystem.reset();

        if (ImGui::GetCurrentContext())
        {
            ImGui::DestroyContext();
        }

        if (editorWindow) editorWindow->Destroy();
        editorWindow.reset();
    }

    void BeginImGuiFrame()
    {
        ImGuiIO& io = ImGui::GetIO();

        const Size2i windowSize = editorWindow->GetSize();

        io.DisplaySize = ImVec2((float32)windowSize.width, (float32)windowSize.height);
        io.DeltaTime = (float32)clock.Delta();

        const Input::Mouse mouse = inputSystem->GetMouse();

        io.AddMousePosEvent((float32)mouse.GetX(), (float32)mouse.GetY());

        if (mouse.ButtonPressed(MouseButtons::LEFT)) io.AddMouseButtonEvent(0, true);
        if (mouse.ButtonReleased(MouseButtons::LEFT)) io.AddMouseButtonEvent(0, false);

        if (mouse.ButtonPressed(MouseButtons::RIGHT)) io.AddMouseButtonEvent(1, true);
        if (mouse.ButtonReleased(MouseButtons::RIGHT)) io.AddMouseButtonEvent(1, false);

        if (mouse.ButtonPressed(MouseButtons::MIDDLE)) io.AddMouseButtonEvent(2, true);
        if (mouse.ButtonReleased(MouseButtons::MIDDLE)) io.AddMouseButtonEvent(2, false);

        if (mouse.ButtonPressed(MouseButtons::X1)) io.AddMouseButtonEvent(3, true);
        if (mouse.ButtonReleased(MouseButtons::X1)) io.AddMouseButtonEvent(3, false);

        if (mouse.ButtonPressed(MouseButtons::X2)) io.AddMouseButtonEvent(4, true);
        if (mouse.ButtonReleased(MouseButtons::X2)) io.AddMouseButtonEvent(4, false);

        const int32 wheelDelta = mouse.GetWheelDelta();

        if (wheelDelta != 0)
        {
            io.AddMouseWheelEvent(0.0f, (float32)wheelDelta / 120.0f);
        }

        //ImGui::NewFrame();
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
