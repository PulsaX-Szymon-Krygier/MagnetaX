// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <MX/Core/Math/Matrix.h>
#include <MX/Core/Math/Size.h>
#include <MX/Core/Math/Vector.h>

class ThinGeometryPreserver
{
public:
    static Matrix4f ComputeRasterModel(const Matrix4f& model, const Vector3f& localBoundsMin,
        const Vector3f& localBoundsMax, const Matrix4f& viewProj, const Size2i& renderSize);
};
