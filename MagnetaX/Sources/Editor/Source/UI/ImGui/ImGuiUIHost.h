// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Graphics/Renderer/UI/UITexture.h>
#include "../AbstractUIHost.h"

class AbstractUIRenderer;

class ImGuiUIHost : public AbstractUIHost
{
public:
    bool Create(const UIHostCreateInfo& createInfo) override;
    void Destroy() override;

    void BeginFrame(float64 deltaTime) override;
    void EndFrame() override;
    void SetFocus(bool focused) override;

private:
    AbstractUIRenderer* renderer = nullptr;
    AbstractWindow* window = nullptr;
    InputSystem* input = nullptr;

    UITextureHandle fontTexture;
};
