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
#include <iostream>

namespace
{
    // Temp place, I will move this
    // to other file later...
    ImGuiKey ToImGuiKey(KeyboardKeys key)
    {
        switch (key)
        {
        // Verify later if correct, generated!
        case KeyboardKeys::TAB: return ImGuiKey_Tab;
        case KeyboardKeys::ARROW_LEFT: return ImGuiKey_LeftArrow;
        case KeyboardKeys::ARROW_RIGHT: return ImGuiKey_RightArrow;
        case KeyboardKeys::ARROW_UP: return ImGuiKey_UpArrow;
        case KeyboardKeys::ARROW_DOWN: return ImGuiKey_DownArrow;
        case KeyboardKeys::PAGE_UP: return ImGuiKey_PageUp;
        case KeyboardKeys::PAGE_DOWN: return ImGuiKey_PageDown;
        case KeyboardKeys::HOME: return ImGuiKey_Home;
        case KeyboardKeys::END: return ImGuiKey_End;
        case KeyboardKeys::INSERT: return ImGuiKey_Insert;
        case KeyboardKeys::DEL: return ImGuiKey_Delete;
        case KeyboardKeys::BACKSPACE: return ImGuiKey_Backspace;
        case KeyboardKeys::SPACE: return ImGuiKey_Space;
        case KeyboardKeys::ENTER: return ImGuiKey_Enter;
        case KeyboardKeys::ESCAPE: return ImGuiKey_Escape;

        case KeyboardKeys::LEFT_CTRL: return ImGuiKey_LeftCtrl;
        case KeyboardKeys::LEFT_SHIFT: return ImGuiKey_LeftShift;
        case KeyboardKeys::LEFT_ALT: return ImGuiKey_LeftAlt;
        case KeyboardKeys::LEFT_SUPER: return ImGuiKey_LeftSuper;
        case KeyboardKeys::RIGHT_CTRL: return ImGuiKey_RightCtrl;
        case KeyboardKeys::RIGHT_SHIFT: return ImGuiKey_RightShift;
        case KeyboardKeys::RIGHT_ALT: return ImGuiKey_RightAlt;
        case KeyboardKeys::RIGHT_SUPER: return ImGuiKey_RightSuper;

        case KeyboardKeys::A: return ImGuiKey_A;
        case KeyboardKeys::B: return ImGuiKey_B;
        case KeyboardKeys::C: return ImGuiKey_C;
        case KeyboardKeys::D: return ImGuiKey_D;
        case KeyboardKeys::E: return ImGuiKey_E;
        case KeyboardKeys::F: return ImGuiKey_F;
        case KeyboardKeys::G: return ImGuiKey_G;
        case KeyboardKeys::H: return ImGuiKey_H;
        case KeyboardKeys::I: return ImGuiKey_I;
        case KeyboardKeys::J: return ImGuiKey_J;
        case KeyboardKeys::K: return ImGuiKey_K;
        case KeyboardKeys::L: return ImGuiKey_L;
        case KeyboardKeys::M: return ImGuiKey_M;
        case KeyboardKeys::N: return ImGuiKey_N;
        case KeyboardKeys::O: return ImGuiKey_O;
        case KeyboardKeys::P: return ImGuiKey_P;
        case KeyboardKeys::Q: return ImGuiKey_Q;
        case KeyboardKeys::R: return ImGuiKey_R;
        case KeyboardKeys::S: return ImGuiKey_S;
        case KeyboardKeys::T: return ImGuiKey_T;
        case KeyboardKeys::U: return ImGuiKey_U;
        case KeyboardKeys::V: return ImGuiKey_V;
        case KeyboardKeys::W: return ImGuiKey_W;
        case KeyboardKeys::X: return ImGuiKey_X;
        case KeyboardKeys::Y: return ImGuiKey_Y;
        case KeyboardKeys::Z: return ImGuiKey_Z;

        case KeyboardKeys::DIGIT_0: return ImGuiKey_0;
        case KeyboardKeys::DIGIT_1: return ImGuiKey_1;
        case KeyboardKeys::DIGIT_2: return ImGuiKey_2;
        case KeyboardKeys::DIGIT_3: return ImGuiKey_3;
        case KeyboardKeys::DIGIT_4: return ImGuiKey_4;
        case KeyboardKeys::DIGIT_5: return ImGuiKey_5;
        case KeyboardKeys::DIGIT_6: return ImGuiKey_6;
        case KeyboardKeys::DIGIT_7: return ImGuiKey_7;
        case KeyboardKeys::DIGIT_8: return ImGuiKey_8;
        case KeyboardKeys::DIGIT_9: return ImGuiKey_9;

        case KeyboardKeys::F1: return ImGuiKey_F1;
        case KeyboardKeys::F2: return ImGuiKey_F2;
        case KeyboardKeys::F3: return ImGuiKey_F3;
        case KeyboardKeys::F4: return ImGuiKey_F4;
        case KeyboardKeys::F5: return ImGuiKey_F5;
        case KeyboardKeys::F6: return ImGuiKey_F6;
        case KeyboardKeys::F7: return ImGuiKey_F7;
        case KeyboardKeys::F8: return ImGuiKey_F8;
        case KeyboardKeys::F9: return ImGuiKey_F9;
        case KeyboardKeys::F10: return ImGuiKey_F10;
        case KeyboardKeys::F11: return ImGuiKey_F11;
        case KeyboardKeys::F12: return ImGuiKey_F12;

        case KeyboardKeys::APOSTROPHE: return ImGuiKey_Apostrophe;
        case KeyboardKeys::COMMA: return ImGuiKey_Comma;
        case KeyboardKeys::MINUS: return ImGuiKey_Minus;
        case KeyboardKeys::PERIOD: return ImGuiKey_Period;
        case KeyboardKeys::SLASH: return ImGuiKey_Slash;
        case KeyboardKeys::SEMICOLON: return ImGuiKey_Semicolon;
        case KeyboardKeys::EQUAL: return ImGuiKey_Equal;
        case KeyboardKeys::LEFT_BRACKET: return ImGuiKey_LeftBracket;
        case KeyboardKeys::BACKSLASH: return ImGuiKey_Backslash;
        case KeyboardKeys::RIGHT_BRACKET: return ImGuiKey_RightBracket;
        case KeyboardKeys::GRAVE: return ImGuiKey_GraveAccent;

        case KeyboardKeys::CAPS_LOCK: return ImGuiKey_CapsLock;
        case KeyboardKeys::SCROLL_LOCK: return ImGuiKey_ScrollLock;
        case KeyboardKeys::NUM_LOCK: return ImGuiKey_NumLock;
        case KeyboardKeys::PRINT_SCREEN: return ImGuiKey_PrintScreen;
        case KeyboardKeys::PAUSE: return ImGuiKey_Pause;

        case KeyboardKeys::NUMPAD_0: return ImGuiKey_Keypad0;
        case KeyboardKeys::NUMPAD_1: return ImGuiKey_Keypad1;
        case KeyboardKeys::NUMPAD_2: return ImGuiKey_Keypad2;
        case KeyboardKeys::NUMPAD_3: return ImGuiKey_Keypad3;
        case KeyboardKeys::NUMPAD_4: return ImGuiKey_Keypad4;
        case KeyboardKeys::NUMPAD_5: return ImGuiKey_Keypad5;
        case KeyboardKeys::NUMPAD_6: return ImGuiKey_Keypad6;
        case KeyboardKeys::NUMPAD_7: return ImGuiKey_Keypad7;
        case KeyboardKeys::NUMPAD_8: return ImGuiKey_Keypad8;
        case KeyboardKeys::NUMPAD_9: return ImGuiKey_Keypad9;
        case KeyboardKeys::NUMPAD_DECIMAL: return ImGuiKey_KeypadDecimal;
        case KeyboardKeys::NUMPAD_DIVIDE: return ImGuiKey_KeypadDivide;
        case KeyboardKeys::NUMPAD_MULTIPLY: return ImGuiKey_KeypadMultiply;
        case KeyboardKeys::NUMPAD_SUBTRACT: return ImGuiKey_KeypadSubtract;
        case KeyboardKeys::NUMPAD_ADD: return ImGuiKey_KeypadAdd;
        case KeyboardKeys::NUMPAD_ENTER: return ImGuiKey_KeypadEnter;

        default:
            return ImGuiKey_None;
        }
    }
}

struct EditorApp::EditorAppImpl
{
    std::unique_ptr<AbstractWindow> editorWindow;
    std::unique_ptr<AbstractGraphicsSystem> graphicsSystem;
    std::unique_ptr<InputSystem> inputSystem;

    Scene editorScene;

    Clock clock;

    bool wantsExit = false;

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

        UpdateImGuiKeyboard();

        ImGui::NewFrame();
    }

    void UpdateImGuiKeyboard()
    {
        ImGuiIO& io = ImGui::GetIO();
        const Input::Keyboard keyboard = inputSystem->GetKeyboard();

        for (usize i = (usize)KeyboardKeys::UNKNOWN + 1; i < (usize)KeyboardKeys::COUNT; ++i)
        {
            const KeyboardKeys key = (KeyboardKeys)i;
            const ImGuiKey imguiKey = ToImGuiKey(key);

            if (imguiKey == ImGuiKey_None) continue;

            if (keyboard.KeyPressed(key)) io.AddKeyEvent(imguiKey, true);
            if (keyboard.KeyReleased(key)) io.AddKeyEvent(imguiKey, false);
        }

        const bool ctrl = keyboard.KeyDown(KeyboardKeys::LEFT_CTRL) || keyboard.KeyDown(KeyboardKeys::RIGHT_CTRL);
        const bool shift = keyboard.KeyDown(KeyboardKeys::LEFT_SHIFT) || keyboard.KeyDown(KeyboardKeys::RIGHT_SHIFT);
        const bool alt = keyboard.KeyDown(KeyboardKeys::LEFT_ALT) || keyboard.KeyDown(KeyboardKeys::RIGHT_ALT);
        const bool super = keyboard.KeyDown(KeyboardKeys::LEFT_SUPER) || keyboard.KeyDown(KeyboardKeys::RIGHT_SUPER);

        io.AddKeyEvent(ImGuiMod_Ctrl, ctrl);
        io.AddKeyEvent(ImGuiMod_Shift, shift);
        io.AddKeyEvent(ImGuiMod_Alt, alt);
        io.AddKeyEvent(ImGuiMod_Super, super);
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
