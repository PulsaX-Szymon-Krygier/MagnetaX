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
#include <MX/Graphics/Renderer/UI/UIDrawData.h>
#include "ImGui/ImGuiAdapter.h"
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

    UITextureHandle uiFontTexture;
    UIDrawData uiDrawData;

    unsigned char* imguiFontPixels = nullptr;
    int32 imguiFontWidth = 0;
    int32 imguiFontHeight = 0;

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
        io.BackendPlatformName = "MagnetaX";

        io.Fonts->GetTexDataAsRGBA32(&imguiFontPixels, &imguiFontWidth, &imguiFontHeight);

        UITextureCreateInfo fontTextureInfo{};
        fontTextureInfo.pixels = imguiFontPixels;
        fontTextureInfo.width = (uint32)imguiFontWidth;
        fontTextureInfo.height = (uint32)imguiFontHeight;
        fontTextureInfo.format = ImageFormat::RGBA8_UNORM;

        uiFontTexture = graphicsSystem->GetUIRenderer().CreateTexture(fontTextureInfo);

        if (!uiFontTexture)
        {
            Destroy();
            return false;
        }

        io.Fonts->SetTexID((ImTextureID)uiFontTexture.id);

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

        BeginImGuiFrame();

        ImGui::Begin("Hierarchy");
        ImGui::TextUnformatted("MagnetaX Editor");
        ImGui::End();

        ImGui::Render();

        const ImDrawData* drawData = ImGui::GetDrawData();

        if (drawData) ImGuiAdapter::FromImGuiDrawData(*drawData, uiDrawData);

        UIRenderData uiData{};
        graphicsSystem->RenderScene(&editorScene, nullptr, uiData);
    }

    void Destroy()
    {
        if (graphicsSystem)
        {
            if (uiFontTexture)
            {
                graphicsSystem->GetUIRenderer().DestroyTexture(uiFontTexture);
                uiFontTexture = {};
            }

            graphicsSystem->Destroy();
        }

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

        ImGuiAdapter::ToImGuiInput(*inputSystem);

        ImGui::NewFrame();
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
        case WindowEventType::FOCUS_GAINED:
        {
            if (ImGui::GetCurrentContext())
            {
                ImGui::GetIO().AddFocusEvent(true);
            }

            break;
        }
        case WindowEventType::FOCUS_LOST:
        {
            if (ImGui::GetCurrentContext())
            {
                ImGui::GetIO().AddFocusEvent(false);
            }

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
