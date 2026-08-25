// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "../CoreMinimal.h"
#include "MathConst.h"

struct MathUtil
{
    static constexpr float32 DegToRad(float32 deg) { return deg * MX_MATH_DEG2RAD; }
    static constexpr float32 RadToDeg(float32 rad) { return rad * MX_MATH_RAD2DEG; }

    template<typename T>
    static constexpr T Lerp(const T& a, const T& b, float32 t)
    {
        return a + (b - a) * t;
    }
};
