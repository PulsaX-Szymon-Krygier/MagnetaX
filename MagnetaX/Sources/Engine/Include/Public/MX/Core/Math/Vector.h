// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "../CoreMinimal.h"

struct Vector2f
{
    float32 x{ 0.0f };
    float32 y{ 0.0f };

    Vector2f() = default;
    Vector2f(float32 value);
    Vector2f(float32 _x, float32 _y);

    void Set(float32 value);
    void Set(float32 _x, float32 _y);

    float32 Length() const;
    float32 LengthSquared() const;

    void Negate();
    Vector2f Negated() const;
    static Vector2f Negated(const Vector2f& vector);

    void Normalize();
    Vector2f Normalized() const;
    static Vector2f Normalized(const Vector2f& vector);

    static float32 Dot(const Vector2f& a, const Vector2f& b);
    static float32 Cross(const Vector2f& a, const Vector2f& b);

    static float32 Distance(const Vector2f& p, const Vector2f& q);
    static float32 DistanceSquared(const Vector2f& p, const Vector2f& q);

    Vector2f operator-() const;
    Vector2f operator+(const Vector2f& r) const;
    Vector2f operator-(const Vector2f& r) const;
    Vector2f operator*(const Vector2f& r) const;
    Vector2f operator*(float32 s) const;
    Vector2f operator/(float32 s) const;

    Vector2f& operator+=(const Vector2f& r);
    Vector2f& operator-=(const Vector2f& r);
    Vector2f& operator*=(const Vector2f& r);
    Vector2f& operator*=(float32 s);
    Vector2f& operator/=(float32 s);

    bool operator==(const Vector2f& r) const;
    bool operator!=(const Vector2f& r) const;
};

Vector2f operator*(float32 s, const Vector2f& v);

struct Vector3f
{
    float32 x{ 0.0f };
    float32 y{ 0.0f };
    float32 z{ 0.0f };

    Vector3f() = default;
    Vector3f(float32 value);
    Vector3f(const Vector2f& vector2, float32 _z);
    Vector3f(float32 _x, float32 _y, float32 _z);

    void Set(float32 value);
    void Set(const Vector2f& vector2, float32 _z);
    void Set(float32 _x, float32 _y, float32 _z);

    float32 Length() const;
    float32 LengthSquared() const;

    void Negate();
    Vector3f Negated() const;
    static Vector3f Negated(const Vector3f& vector);

    void Normalize();
    Vector3f Normalized() const;
    static Vector3f Normalized(const Vector3f& vector);

    static float32 Dot(const Vector3f& a, const Vector3f& b);
    static Vector3f Cross(const Vector3f& a, const Vector3f& b);

    static float32 Distance(const Vector3f& p, const Vector3f& q);
    static float32 DistanceSquared(const Vector3f& p, const Vector3f& q);

    Vector3f operator-() const;
    Vector3f operator+(const Vector3f& r) const;
    Vector3f operator-(const Vector3f& r) const;
    Vector3f operator*(const Vector3f& r) const;
    Vector3f operator*(float32 s) const;
    Vector3f operator/(float32 s) const;

    Vector3f& operator+=(const Vector3f& r);
    Vector3f& operator-=(const Vector3f& r);
    Vector3f& operator*=(const Vector3f& r);
    Vector3f& operator*=(float32 s);
    Vector3f& operator/=(float32 s);

    bool operator==(const Vector3f& r) const;
    bool operator!=(const Vector3f& r) const;
};

Vector3f operator*(float32 s, const Vector3f& v);

struct Vector4f
{
    float32 x{ 0.0f };
    float32 y{ 0.0f };
    float32 z{ 0.0f };
    float32 w{ 1.0f };

    Vector4f() = default;
    Vector4f(float32 value);
    Vector4f(const Vector2f& vector2, float32 _z, float32 _w);
    Vector4f(const Vector3f& vector3, float32 _w);
    Vector4f(float32 _x, float32 _y, float32 _z, float32 _w);

    void Set(float32 value);
    void Set(const Vector2f& vector2, float32 _z, float32 _w);
    void Set(const Vector3f& vector3, float32 _w);
    void Set(float32 _x, float32 _y, float32 _z, float32 _w);

    float32 Length() const;
    float32 LengthSquared() const;

    void Negate();
    Vector4f Negated() const;
    static Vector4f Negated(const Vector4f& vector);

    void Normalize();
    Vector4f Normalized() const;
    static Vector4f Normalized(const Vector4f& vector);

    static float32 Dot(const Vector4f& a, const Vector4f& b);

    static float32 Distance(const Vector4f& p, const Vector4f& q);
    static float32 DistanceSquared(const Vector4f& p, const Vector4f& q);

    Vector4f operator-() const;
    Vector4f operator+(const Vector4f& r) const;
    Vector4f operator-(const Vector4f& r) const;
    Vector4f operator*(const Vector4f& r) const;
    Vector4f operator*(float32 s) const;
    Vector4f operator/(float32 s) const;

    Vector4f& operator+=(const Vector4f& r);
    Vector4f& operator-=(const Vector4f& r);
    Vector4f& operator*=(const Vector4f& r);
    Vector4f& operator*=(float32 s);
    Vector4f& operator/=(float32 s);

    bool operator==(const Vector4f& r) const;
    bool operator!=(const Vector4f& r) const;
};

Vector4f operator*(float32 s, const Vector4f& v);
