// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "ImGuiEditorUI.h"
#include <imgui.h>

void ImGuiEditorUI::Draw()
{
    ImGui::Begin("Test");
    ImGui::TextUnformatted("TestText");
    ImGui::End();
}
