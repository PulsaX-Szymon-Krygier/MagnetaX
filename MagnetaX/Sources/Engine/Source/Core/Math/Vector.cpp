// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Core/Math/Vector.h>
#include <MX/Core/Math/MathConst.h>
#include <cmath>

Vector2f::Vector2f(float32 value) : x(value), y(value) {}

Vector2f::Vector2f(float32 _x, float32 _y) : x(_x), y(_y) {}

void Vector2f::Set(float32 value)
{
    x = value;
    y = value;
}

void Vector2f::Set(float32 _x, float32 _y)
{
    x = _x;
    y = _y;
}

float32 Vector2f::Length() const
{
    return static_cast<float32>(std::sqrt(LengthSquared()));
}

float32 Vector2f::LengthSquared() const
{
    return (x * x) + (y * y);
}

void Vector2f::Negate()
{
    x = -x;
    y = -y;
}

Vector2f Vector2f::Negated() const
{
    Vector2f v = *this;
    v.Negate();

    return v;
}

Vector2f Vector2f::Negated(const Vector2f& vector)
{
    return vector.Negated();
}

void Vector2f::Normalize()
{
    const float32 lenSq = LengthSquared();
    if (lenSq <= MX_MATH_EPSILON_SQUARED) return;

    const float32 invLen = 1.0f / static_cast<float32>(std::sqrt(lenSq));

    x *= invLen;
    y *= invLen;
}

Vector2f Vector2f::Normalized() const
{
    Vector2f v = *this;
    v.Normalize();

    return v;
}

Vector2f Vector2f::Normalized(const Vector2f& vector)
{
    return vector.Normalized();
}

float32 Vector2f::Dot(const Vector2f& a, const Vector2f& b)
{
    return (a.x * b.x) + (a.y * b.y);
}

float32 Vector2f::Cross(const Vector2f& a, const Vector2f& b)
{
    return (a.x * b.y) - (a.y * b.x);
}

float32 Vector2f::Distance(const Vector2f& p, const Vector2f& q)
{
    return static_cast<float32>(std::sqrt(DistanceSquared(p, q)));
}

float32 Vector2f::DistanceSquared(const Vector2f& p, const Vector2f& q)
{
    const float32 dx = p.x - q.x;
    const float32 dy = p.y - q.y;

    return (dx * dx) + (dy * dy);
}

Vector2f Vector2f::operator-() const
{
    return Vector2f(-x, -y);
}

Vector2f Vector2f::operator+(const Vector2f& r) const
{
    return Vector2f(x + r.x, y + r.y);
}

Vector2f Vector2f::operator-(const Vector2f& r) const
{
    return Vector2f(x - r.x, y - r.y);
}

Vector2f Vector2f::operator*(const Vector2f& r) const
{
    return Vector2f(x * r.x, y * r.y);
}

Vector2f Vector2f::operator*(float32 s) const
{
    return Vector2f(x * s, y * s);
}

Vector2f Vector2f::operator/(float32 s) const
{
    if (std::fabs(s) <= MX_MATH_EPSILON) return Vector2f(0.0f);

    const float32 inv = 1.0f / s;

    return Vector2f(x * inv, y * inv);
}

Vector2f& Vector2f::operator+=(const Vector2f& r)
{
    x += r.x;
    y += r.y;

    return *this;
}

Vector2f& Vector2f::operator-=(const Vector2f& r)
{
    x -= r.x;
    y -= r.y;

    return *this;
}

Vector2f& Vector2f::operator*=(const Vector2f& r)
{
    x *= r.x;
    y *= r.y;

    return *this;
}

Vector2f& Vector2f::operator*=(float32 s)
{
    x *= s;
    y *= s;

    return *this;
}

Vector2f& Vector2f::operator/=(float32 s)
{
    if (std::fabs(s) <= MX_MATH_EPSILON)
    {
        x = 0.0f;
        y = 0.0f;

        return *this;
    }

    const float32 inv = 1.0f / s;

    x *= inv;
    y *= inv;

    return *this;
}

bool Vector2f::operator==(const Vector2f& r) const
{
    return x == r.x && y == r.y;
}

bool Vector2f::operator!=(const Vector2f& r) const
{
    return !(*this == r);
}

Vector2f operator*(float32 s, const Vector2f& v)
{
    return v * s;
}

Vector3f::Vector3f(float32 value) : x(value), y(value), z(value) {}

Vector3f::Vector3f(const Vector2f& vector2, float32 _z) : x(vector2.x), y(vector2.y), z(_z) {}

Vector3f::Vector3f(float32 _x, float32 _y, float32 _z) : x(_x), y(_y), z(_z) {}

void Vector3f::Set(float32 value)
{
    x = value;
    y = value;
    z = value;
}

void Vector3f::Set(const Vector2f& vector2, float32 _z)
{
    x = vector2.x;
    y = vector2.y;
    z = _z;
}

void Vector3f::Set(float32 _x, float32 _y, float32 _z)
{
    x = _x;
    y = _y;
    z = _z;
}

float32 Vector3f::Length() const
{
    return static_cast<float32>(std::sqrt(LengthSquared()));
}

float32 Vector3f::LengthSquared() const
{
    return (x * x) + (y * y) + (z * z);
}

void Vector3f::Negate()
{
    x = -x;
    y = -y;
    z = -z;
}

Vector3f Vector3f::Negated() const
{
    Vector3f v = *this;
    v.Negate();

    return v;
}

Vector3f Vector3f::Negated(const Vector3f& vector)
{
    return vector.Negated();
}

void Vector3f::Normalize()
{
    const float32 lenSq = LengthSquared();
    if (lenSq <= MX_MATH_EPSILON_SQUARED) return;

    const float32 invLen = 1.0f / static_cast<float32>(std::sqrt(lenSq));

    x *= invLen;
    y *= invLen;
    z *= invLen;
}

Vector3f Vector3f::Normalized() const
{
    Vector3f v = *this;
    v.Normalize();

    return v;
}

Vector3f Vector3f::Normalized(const Vector3f& vector)
{
    return vector.Normalized();
}

float32 Vector3f::Dot(const Vector3f& a, const Vector3f& b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

Vector3f Vector3f::Cross(const Vector3f& a, const Vector3f& b)
{
    return Vector3f(
        (a.y * b.z) - (a.z * b.y),
        (a.z * b.x) - (a.x * b.z),
        (a.x * b.y) - (a.y * b.x)
    );
}

float32 Vector3f::Distance(const Vector3f& p, const Vector3f& q)
{
    return static_cast<float32>(std::sqrt(DistanceSquared(p, q)));
}

float32 Vector3f::DistanceSquared(const Vector3f& p, const Vector3f& q)
{
    const float32 dx = p.x - q.x;
    const float32 dy = p.y - q.y;
    const float32 dz = p.z - q.z;

    return (dx * dx) + (dy * dy) + (dz * dz);
}

Vector3f Vector3f::operator-() const
{
    return Vector3f(-x, -y, -z);
}

Vector3f Vector3f::operator+(const Vector3f& r) const
{
    return Vector3f(x + r.x, y + r.y, z + r.z);
}

Vector3f Vector3f::operator-(const Vector3f& r) const
{
    return Vector3f(x - r.x, y - r.y, z - r.z);
}

Vector3f Vector3f::operator*(const Vector3f& r) const
{
    return Vector3f(x * r.x, y * r.y, z * r.z);
}

Vector3f Vector3f::operator*(float32 s) const
{
    return Vector3f(x * s, y * s, z * s);
}

Vector3f Vector3f::operator/(float32 s) const
{
    if (std::fabs(s) <= MX_MATH_EPSILON) return Vector3f(0.0f);

    const float32 inv = 1.0f / s;

    return Vector3f(x * inv, y * inv, z * inv);
}

Vector3f& Vector3f::operator+=(const Vector3f& r)
{
    x += r.x;
    y += r.y;
    z += r.z;

    return *this;
}

Vector3f& Vector3f::operator-=(const Vector3f& r)
{
    x -= r.x;
    y -= r.y;
    z -= r.z;

    return *this;
}

Vector3f& Vector3f::operator*=(const Vector3f& r)
{
    x *= r.x;
    y *= r.y;
    z *= r.z;

    return *this;
}

Vector3f& Vector3f::operator*=(float32 s)
{
    x *= s;
    y *= s;
    z *= s;

    return *this;
}

Vector3f& Vector3f::operator/=(float32 s)
{
    if (std::fabs(s) <= MX_MATH_EPSILON)
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;

        return *this;
    }

    const float32 inv = 1.0f / s;

    x *= inv;
    y *= inv;
    z *= inv;

    return *this;
}

bool Vector3f::operator==(const Vector3f& r) const
{
    return x == r.x && y == r.y && z == r.z;
}

bool Vector3f::operator!=(const Vector3f& r) const
{
    return !(*this == r);
}

Vector3f operator*(float32 s, const Vector3f& v)
{
    return v * s;
}

Vector4f::Vector4f(float32 value) : x(value), y(value), z(value), w(value) {}

Vector4f::Vector4f(const Vector2f& vector2, float32 _z, float32 _w) : x(vector2.x), y(vector2.y), z(_z), w(_w) {}

Vector4f::Vector4f(const Vector3f& vector3, float32 _w) : x(vector3.x), y(vector3.y), z(vector3.z), w(_w) {}

Vector4f::Vector4f(float32 _x, float32 _y, float32 _z, float32 _w) : x(_x), y(_y), z(_z), w(_w) {}

void Vector4f::Set(float32 value)
{
    x = value;
    y = value;
    z = value;
    w = value;
}

void Vector4f::Set(const Vector2f& vector2, float32 _z, float32 _w)
{
    x = vector2.x;
    y = vector2.y;
    z = _z;
    w = _w;
}

void Vector4f::Set(const Vector3f& vector3, float32 _w)
{
    x = vector3.x;
    y = vector3.y;
    z = vector3.z;
    w = _w;
}

void Vector4f::Set(float32 _x, float32 _y, float32 _z, float32 _w)
{
    x = _x;
    y = _y;
    z = _z;
    w = _w;
}

float32 Vector4f::Length() const
{
    return static_cast<float32>(std::sqrt(LengthSquared()));
}

float32 Vector4f::LengthSquared() const
{
    return (x * x) + (y * y) + (z * z) + (w * w);
}

void Vector4f::Negate()
{
    x = -x;
    y = -y;
    z = -z;
    w = -w;
}

Vector4f Vector4f::Negated() const
{
    Vector4f v = *this;
    v.Negate();

    return v;
}

Vector4f Vector4f::Negated(const Vector4f& vector)
{
    return vector.Negated();
}

void Vector4f::Normalize()
{
    const float32 lenSq = LengthSquared();
    if (lenSq <= MX_MATH_EPSILON_SQUARED) return;

    const float32 invLen = 1.0f / static_cast<float32>(std::sqrt(lenSq));

    x *= invLen;
    y *= invLen;
    z *= invLen;
    w *= invLen;
}

Vector4f Vector4f::Normalized() const
{
    Vector4f v = *this;
    v.Normalize();

    return v;
}

Vector4f Vector4f::Normalized(const Vector4f& vector)
{
    return vector.Normalized();
}

float32 Vector4f::Dot(const Vector4f& a, const Vector4f& b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

float32 Vector4f::Distance(const Vector4f& p, const Vector4f& q)
{
    return static_cast<float32>(std::sqrt(DistanceSquared(p, q)));
}

float32 Vector4f::DistanceSquared(const Vector4f& p, const Vector4f& q)
{
    const float32 dx = p.x - q.x;
    const float32 dy = p.y - q.y;
    const float32 dz = p.z - q.z;
    const float32 dw = p.w - q.w;

    return (dx * dx) + (dy * dy) + (dz * dz) + (dw * dw);
}

Vector4f Vector4f::operator-() const
{
    return Vector4f(-x, -y, -z, -w);
}

Vector4f Vector4f::operator+(const Vector4f& r) const
{
    return Vector4f(x + r.x, y + r.y, z + r.z, w + r.w);
}

Vector4f Vector4f::operator-(const Vector4f& r) const
{
    return Vector4f(x - r.x, y - r.y, z - r.z, w - r.w);
}

Vector4f Vector4f::operator*(const Vector4f& r) const
{
    return Vector4f(x * r.x, y * r.y, z * r.z, w * r.w);
}

Vector4f Vector4f::operator*(float32 s) const
{
    return Vector4f(x * s, y * s, z * s, w * s);
}

Vector4f Vector4f::operator/(float32 s) const
{
    if (std::fabs(s) <= MX_MATH_EPSILON) return Vector4f(0.0f);

    const float32 inv = 1.0f / s;

    return Vector4f(x * inv, y * inv, z * inv, w * inv);
}

Vector4f& Vector4f::operator+=(const Vector4f& r)
{
    x += r.x;
    y += r.y;
    z += r.z;
    w += r.w;

    return *this;
}

Vector4f& Vector4f::operator-=(const Vector4f& r)
{
    x -= r.x;
    y -= r.y;
    z -= r.z;
    w -= r.w;

    return *this;
}

Vector4f& Vector4f::operator*=(const Vector4f& r)
{
    x *= r.x;
    y *= r.y;
    z *= r.z;
    w *= r.w;

    return *this;
}

Vector4f& Vector4f::operator*=(float32 s)
{
    x *= s;
    y *= s;
    z *= s;
    w *= s;

    return *this;
}

Vector4f& Vector4f::operator/=(float32 s)
{
    if (std::fabs(s) <= MX_MATH_EPSILON)
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        w = 0.0f;

        return *this;
    }

    const float32 inv = 1.0f / s;

    x *= inv;
    y *= inv;
    z *= inv;
    w *= inv;

    return *this;
}

bool Vector4f::operator==(const Vector4f& r) const
{
    return x == r.x && y == r.y && z == r.z && w == r.w;
}

bool Vector4f::operator!=(const Vector4f& r) const
{
    return !(*this == r);
}

Vector4f operator*(float32 s, const Vector4f& v)
{
    return v * s;
}
