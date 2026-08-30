// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Graphics/Renderer/UI/UITexture.h>

class AbstractUIRenderer;
class AbstractWindow;
class InputSystem;

struct UIHostCreateInfo
{
    AbstractUIRenderer* renderer = nullptr;
    AbstractWindow* window = nullptr;
    InputSystem* input = nullptr;
};

// Because one day we will probably make our
// own GUI system as well using MXEngine UI elements
class AbstractUIHost
{
public:
    virtual ~AbstractUIHost() = default;

    virtual bool Create(const UIHostCreateInfo& createInfo) = 0;
    virtual void Destroy() = 0;

    virtual void BeginFrame(float64 deltaTime) = 0;
    virtual void EndFrame() = 0;
    virtual void SetFocus(bool focused) = 0;
};
