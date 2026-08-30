// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "ImGuiUIHost.h"
#include <MX/Graphics/Renderer/UI/AbstractUIRenderer.h>
#include <MX/Window/AbstractWindow.h>
#include <MX/Input/InputSystem.h>
#include <MX/Graphics/Renderer/UI/AbstractUIRenderer.h>
#include "ImGuiAdapter.h"
#include <imgui.h>

bool ImGuiUIHost::Create(const UIHostCreateInfo& createInfo)
{
    if (!createInfo.renderer || !createInfo.window || !createInfo.input) return false;

    renderer = createInfo.renderer;
    window = createInfo.window;
    input = createInfo.input;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.BackendPlatformName = "MXEditor";

    unsigned char* fontPixels = nullptr;
    int fontWidth = 0;
    int fontHeight = 0;

    io.Fonts->GetTexDataAsAlpha8(&fontPixels, &fontWidth, &fontHeight);

    UITextureCreateInfo fontTextureInfo{};
    fontTextureInfo.pixels = fontPixels;
    fontTextureInfo.width = (uint32)fontWidth;
    fontTextureInfo.height = (uint32)fontHeight;
    fontTextureInfo.format = ImageFormat::R8_UNORM;

    fontTexture = renderer->CreateTexture(fontTextureInfo);

    if (!fontTexture)
    {
        ImGui::DestroyContext();
        return false;
    }

    io.Fonts->SetTexID((ImTextureID)fontTexture.id);

    return true;
}

void ImGuiUIHost::Destroy()
{
    if (fontTexture)
    {
        renderer->DestroyTexture(fontTexture);
        fontTexture = {};
    }

    if (ImGui::GetCurrentContext())
    {
        ImGui::DestroyContext();
    }

    renderer = nullptr;
    window = nullptr;
    input = nullptr;
}

void ImGuiUIHost::BeginFrame(float64 deltaTime)
{
    ImGuiIO& io = ImGui::GetIO();

    const Size2i windowSize = window->GetSize();
    const Size2i surfaceSize = window->GetSurfaceSize();

    io.DisplaySize = ImVec2((float32)windowSize.width, (float32)windowSize.height);
    io.DisplayFramebufferScale = ImVec2(windowSize.width > 0 ? (float32)surfaceSize.width / (float32)windowSize.width : 1.0f,
        windowSize.height > 0 ? (float32)surfaceSize.height / (float32)windowSize.height : 1.0f);

    io.DeltaTime = (float32)deltaTime;

    ImGuiAdapter::ToImGuiInput(*input);

    ImGui::NewFrame();
}

void ImGuiUIHost::EndFrame()
{
    ImGui::Render();

    const ImDrawData* drawData = ImGui::GetDrawData();

    if (drawData) ImGuiAdapter::FromImGuiDrawData(*drawData, renderer->GetDrawData());
}

void ImGuiUIHost::SetFocus(bool focused)
{
    if (!ImGui::GetCurrentContext()) return;

    ImGui::GetIO().AddFocusEvent(focused);
}
