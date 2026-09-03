// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/Math/Matrix.h>
#include <MX/Core/Math/MathUtil.h>

struct CameraComponent
{
    float32 fieldOfView = 60.0f;
    float32 nearPlane = 0.1f;
    float32 farPlane = 1000.0f;
    float32 exposureEV = 0.0f; // Should I move this to GraphicsConfig? TODO: think about this

    Matrix4f GetProjectionMatrix(float32 aspectRatio) const
    {
        return Matrix4f::ProjectionPerspectiveRightHanded(MathUtil::DegToRad(fieldOfView), aspectRatio, nearPlane, farPlane);
    }
};
