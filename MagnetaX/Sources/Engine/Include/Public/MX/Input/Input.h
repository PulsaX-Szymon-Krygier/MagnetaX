// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include "KeyboardState.h"
#include "MouseState.h"

class Input
{
public:
    virtual ~Input() = default;

    class Keyboard
    {
    public:
        explicit Keyboard(const KeyboardState& s) : state(s) {}

        bool KeyDown(KeyboardKeys key) const { return state.keyDown[(usize)key]; }
        bool KeyPressed(KeyboardKeys key) const { return state.keyPressed[(usize)key]; }
        bool KeyReleased(KeyboardKeys key) const { return state.keyReleased[(usize)key]; }

    private:
        const KeyboardState& state;
    };

    class Mouse
    {
    public:
        explicit Mouse(const MouseState& s) : state(s) {}

        bool ButtonDown(MouseButtons b) const { return state.buttonDown[(usize)b]; }
        bool ButtonPressed(MouseButtons b) const { return state.buttonPressed[(usize)b]; }
        bool ButtonReleased(MouseButtons b) const { return state.buttonReleased[(usize)b]; }

        int32 GetX() const { return state.mouseX; }
        int32 GetY() const { return state.mouseY; }
        int32 GetDX() const { return state.mouseDX; }
        int32 GetDY() const { return state.mouseDY; }
        int32 GetWheelDelta() const { return state.wheelDelta; }

    private:
        const MouseState& state;
    };

    Keyboard GetKeyboard() const { return Keyboard(GetKeyboardState()); }
    Mouse GetMouse() const { return Mouse(GetMouseState()); }

private:
    virtual const KeyboardState& GetKeyboardState() const = 0;
    virtual const MouseState& GetMouseState() const = 0;
};
