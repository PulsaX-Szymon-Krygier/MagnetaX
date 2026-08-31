// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Input/Input.h>

class InputFeed : public Input
{
public:
    virtual void BeginFrame() = 0;

    virtual void SetKeyDown(KeyboardKeys key, bool down) = 0;

    virtual void SetMouseButton(MouseButtons button, bool down) = 0;
    virtual void SetMousePosition(int32 x, int32 y) = 0;

    virtual void AddMouseWheel(int32 delta) = 0;

    virtual void Reset() = 0;
};
