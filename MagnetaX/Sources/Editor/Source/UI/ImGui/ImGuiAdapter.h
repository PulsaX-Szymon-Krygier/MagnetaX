// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Input/KeyboardKeys.h>
#include <MX/Graphics/Renderer/UI/UIDrawData.h>
#include <imgui.h>

class InputSystem;

struct ImGuiAdapter
{
    // Local KeyboardKeys to ImGuiKey
    static ImGuiKey ToImGuiKeys(KeyboardKeys key);

    // Local input to ImGui input
    static void ToImGuiInput(const InputSystem& input);

    // ImDrawData to local UIDrawData
    static void FromImGuiDrawData(const ImDrawData& im, UIDrawData& local);
};
