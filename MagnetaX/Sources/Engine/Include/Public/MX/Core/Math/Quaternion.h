// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "../CoreMinimal.h"

struct Matrix4f;
struct Vector3f;

struct Quaternion
{
    float32 x{ 0.0f };
    float32 y{ 0.0f };
    float32 z{ 0.0f };
    float32 w{ 1.0f };

    Quaternion() = default;
    Quaternion(float32 _x, float32 _y, float32 _z, float32 _w);

    static Quaternion Identity();

    float32 Length() const;
    float32 LengthSquared() const;

    void Normalize();
    Quaternion Normalized() const;
    static Quaternion Normalized(const Quaternion& q);

    static Quaternion Multiply(const Quaternion& a, const Quaternion& b);

    static Quaternion FromAxisAngleDegrees(const Vector3f& axis, float32 angleDeg);
    static Quaternion FromAxisAngleRadians(const Vector3f& axis, float32 angleRad);

    static Quaternion FromYawPitchRollDegrees(float32 yawDeg, float32 pitchDeg, float32 rollDeg);
    static Quaternion FromYawPitchRollRadians(float32 yawRad, float32 pitchRad, float32 rollRad);

    static Matrix4f ToMatrix(const Quaternion& q);

    Vector3f RotateVector(const Vector3f& v) const;
    static Vector3f RotateVector(const Quaternion& q, const Vector3f& v);

    Quaternion operator*(const Quaternion& r) const;
    Quaternion& operator*=(const Quaternion& r);

    bool operator==(const Quaternion& r) const;
    bool operator!=(const Quaternion& r) const;
};
