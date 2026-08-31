// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Input/InputSystem.h>

void InputSystem::BeginFrame()
{
    keyboardState.keyPressed.fill(false);
    keyboardState.keyReleased.fill(false);

    mouseState.buttonPressed.fill(false);
    mouseState.buttonReleased.fill(false);

    mouseState.mouseDX = 0;
    mouseState.mouseDY = 0;
    mouseState.wheelDelta = 0;
}

void InputSystem::SetKeyDown(KeyboardKeys key, bool down)
{
    const usize idx = (usize)key;
    if (idx >= (usize)KeyboardKeys::COUNT) return;

    const bool wasDown = keyboardState.keyDown[idx];
    if (wasDown == down) return;

    keyboardState.keyDown[idx] = down;

    if (down) keyboardState.keyPressed[idx] = true;
    else keyboardState.keyReleased[idx] = true;
}

void InputSystem::SetMouseButton(MouseButtons button, bool down)
{
    const usize idx = (usize)button;
    if (idx >= (usize)MouseButtons::COUNT) return;

    const bool wasDown = mouseState.buttonDown[idx];
    if (wasDown == down) return;

    mouseState.buttonDown[idx] = down;

    if (down) mouseState.buttonPressed[idx] = true;
    else mouseState.buttonReleased[idx] = true;
}

void InputSystem::SetMousePosition(int32 x, int32 y)
{
    if (!mouseInitialized)
    {
        mouseState.mouseX = x;
        mouseState.mouseY = y;
        mouseInitialized = true;

        return;
    }

    mouseState.mouseDX += (x - mouseState.mouseX);
    mouseState.mouseDY += (y - mouseState.mouseY);

    mouseState.mouseX = x;
    mouseState.mouseY = y;
}

void InputSystem::AddMouseWheel(int32 delta)
{
    mouseState.wheelDelta += delta;
}

void InputSystem::Reset()
{
    keyboardState = {};
    mouseState = {};
    mouseInitialized = false;
}
