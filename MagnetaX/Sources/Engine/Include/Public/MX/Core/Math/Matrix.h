// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "../CoreMinimal.h"

struct Quaternion;
struct Vector3f;
struct Vector4f;

struct Matrix4f
{
    float32 m00{ 1.0f }, m01{ 0.0f }, m02{ 0.0f }, m03{ 0.0f };
    float32 m10{ 0.0f }, m11{ 1.0f }, m12{ 0.0f }, m13{ 0.0f };
    float32 m20{ 0.0f }, m21{ 0.0f }, m22{ 1.0f }, m23{ 0.0f };
    float32 m30{ 0.0f }, m31{ 0.0f }, m32{ 0.0f }, m33{ 1.0f };

    Matrix4f() = default;

    static Matrix4f Identity();
    void SetIdentity();

    void Transpose();
    Matrix4f Transposed() const;
    static Matrix4f Transposed(const Matrix4f& m);

    void Inverse();
    Matrix4f Inversed() const;
    static Matrix4f Inversed(const Matrix4f& m);

    static Matrix4f Mul(const Matrix4f& a, const Matrix4f& b);

    static Matrix4f FromTranslation(const Vector3f& t);
    static Matrix4f FromScale(const Vector3f& s);
    static Matrix4f FromRotation(const Quaternion& q);

    static Matrix4f ViewLookAtRightHanded(const Vector3f& eye, const Vector3f& target, const Vector3f& up);
    static Matrix4f ViewFromPositionRotation(const Vector3f& position, const Quaternion& rotation);

    static Matrix4f ProjectionPerspectiveRightHanded(float32 fovYRad, float32 aspect, float32 zNear, float32 zFar);
    static Matrix4f ProjectionOrthographicRightHanded(float32 width, float32 height, float32 zNear, float32 zFar);

    static Matrix4f FromTranslationRotationScale(const Vector3f& position, const Quaternion& rotation, const Vector3f& scale);

    Vector4f operator*(const Vector4f& v) const;
    Matrix4f operator*(const Matrix4f& r) const;
    Matrix4f& operator*=(const Matrix4f& r);
};
