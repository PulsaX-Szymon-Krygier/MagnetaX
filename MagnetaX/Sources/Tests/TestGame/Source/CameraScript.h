// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Scripting/Script.h>

class CameraScript : public Script
{
public:
    float32 moveSpeed = 1.5f;
    float32 mouseSens = 0.2f;

private:
    float32 yaw = 0.0f;
    float32 pitch = 0.0f;

    void OnUpdate(float64 deltaTime) override;
};
