// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/CoreMinimal.h>
#include <MX/Core/Math/Matrix.h>
#include <MX/Core/Math/Vector.h>

struct RenderViewData
{
    Matrix4f view = Matrix4f::Identity();
    Matrix4f proj = Matrix4f::Identity();
    Matrix4f viewProj = Matrix4f::Identity();
    Matrix4f invViewProj = Matrix4f::Identity();

    Matrix4f jitteredProj = Matrix4f::Identity();
    Matrix4f jitteredViewProj = Matrix4f::Identity();
    Matrix4f invJitteredViewProj = Matrix4f::Identity();

    Vector2f jitter{ 0.0f };

    uint32 cameraId = 0;
    Vector3f position{ 0.0f };
    float32 nearPlane = 0.1f;
    float32 farPlane = 1000.0f;
    float32 exposureEV = 0.0f;

    bool valid = false;
};
