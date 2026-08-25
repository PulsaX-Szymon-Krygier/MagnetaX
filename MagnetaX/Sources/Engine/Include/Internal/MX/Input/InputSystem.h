// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include "InputFeed.h"

class InputSystem final : public InputFeed
{
public:
    /* InputFeed */
    void BeginFrame() override;

    void SetKeyDown(KeyboardKeys key, bool down) override;

    void SetMouseButton(MouseButtons button, bool down) override;
    void SetMousePosition(int32 x, int32 y) override;

    void AddMouseWheel(int32 delta) override;

private:
    /* Input */
    const KeyboardState& GetKeyboardState() const override { return keyboardState; }
    const MouseState& GetMouseState() const override { return mouseState; }

    /* InputSystem */
    KeyboardState keyboardState{};
    MouseState mouseState{};

    bool mouseInitialized = false;
};
