// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "ImGuiAdapter.h"
#include <MX/Input/InputSystem.h>

ImGuiKey ImGuiAdapter::ToImGuiKeys(KeyboardKeys key)
{
    switch (key)
    {
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

void ImGuiAdapter::ToImGuiInput(const InputSystem& input)
{
    ImGuiIO& io = ImGui::GetIO();

    const Input::Mouse mouse = input.GetMouse();

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

    const Input::Keyboard keyboard = input.GetKeyboard();

    for (usize i = (usize)KeyboardKeys::UNKNOWN + 1; i < (usize)KeyboardKeys::COUNT; ++i)
    {
        const KeyboardKeys key = (KeyboardKeys)i;
        const ImGuiKey imKey = ToImGuiKeys(key);

        if (imKey == ImGuiKey_None) continue;

        if (keyboard.KeyPressed(key)) io.AddKeyEvent(imKey, true);
        if (keyboard.KeyReleased(key)) io.AddKeyEvent(imKey, false);
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

void ImGuiAdapter::FromImGuiDrawData(const ImDrawData& im, UIDrawData& local)
{
    local.vertices.clear();
    local.indices.clear();
    local.commands.clear();

    local.vertices.reserve(im.TotalVtxCount);
    local.indices.reserve(im.TotalIdxCount);

    const ImVec2 displayPos = im.DisplayPos;
    const ImVec2 framebufferScale = im.FramebufferScale;

    uint32 vertexOffset = 0;
    uint32 indexOffset = 0;

    for (int32 listIndex = 0; listIndex < im.CmdListsCount; ++listIndex)
    {
        const ImDrawList* list = im.CmdLists[listIndex];

        for (const ImDrawVert& vertex : list->VtxBuffer)
        {
            const ImVec4 color = ImGui::ColorConvertU32ToFloat4(vertex.col);

            UIDrawVertex drawVertex{};
            drawVertex.position = Vector2f((vertex.pos.x - displayPos.x) * framebufferScale.x, (vertex.pos.y - displayPos.y) * framebufferScale.y);
            drawVertex.uv = Vector2f(vertex.uv.x, vertex.uv.y);
            drawVertex.color = Vector4f(color.x, color.y, color.z, color.w);

            local.vertices.push_back(drawVertex);
        }

        for (ImDrawIdx index : list->IdxBuffer)
        {
            local.indices.push_back((uint32)index);
        }

        for (const ImDrawCmd& command : list->CmdBuffer)
        {
            if (command.UserCallback) continue;

            UIDrawCommand drawCommand{};
            drawCommand.clipMin = Vector2f((command.ClipRect.x - displayPos.x) * framebufferScale.x, (command.ClipRect.y - displayPos.y) * framebufferScale.y);
            drawCommand.clipMax = Vector2f((command.ClipRect.z - displayPos.x) * framebufferScale.x, (command.ClipRect.w - displayPos.y) * framebufferScale.y);
            drawCommand.texture = UITextureHandle{ (uint64)command.GetTexID() };
            drawCommand.indexOffset = indexOffset + command.IdxOffset;
            drawCommand.indexCount = command.ElemCount;
            drawCommand.vertexOffset = vertexOffset + command.VtxOffset;

            local.commands.push_back(drawCommand);
        }

        vertexOffset += (uint32)list->VtxBuffer.Size;
        indexOffset += (uint32)list->IdxBuffer.Size;
    }
}
