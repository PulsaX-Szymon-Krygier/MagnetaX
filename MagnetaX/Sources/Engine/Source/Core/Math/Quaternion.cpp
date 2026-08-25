// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Core/Math/Quaternion.h>
#include <MX/Core/Math/MathUtil.h>
#include <MX/Core/Math/Matrix.h>
#include <MX/Core/Math/Vector.h>
#include <cmath>

Quaternion::Quaternion(float32 _x, float32 _y, float32 _z, float32 _w) : x(_x), y(_y), z(_z), w(_w) {}

Quaternion Quaternion::Identity()
{
    return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
}

float32 Quaternion::Length() const
{
    return static_cast<float32>(std::sqrt(LengthSquared()));
}

float32 Quaternion::LengthSquared() const
{
    return x * x + y * y + z * z + w * w;
}

void Quaternion::Normalize()
{
    const float32 length = Length();

    if (length <= MX_MATH_EPSILON)
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        w = 1.0f;
        return;
    }

    const float32 invLength = 1.0f / length;

    x *= invLength;
    y *= invLength;
    z *= invLength;
    w *= invLength;
}

Quaternion Quaternion::Normalized() const
{
    Quaternion q = *this;
    q.Normalize();

    return q;
}

Quaternion Quaternion::Normalized(const Quaternion& q)
{
    return q.Normalized();
}

Quaternion Quaternion::Multiply(const Quaternion& a, const Quaternion& b)
{
    return Quaternion(
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    );
}

Quaternion Quaternion::FromAxisAngleDegrees(const Vector3f& axis, float32 angleDeg)
{
    return FromAxisAngleRadians(axis, MathUtil::DegToRad(angleDeg));
}

Quaternion Quaternion::FromAxisAngleRadians(const Vector3f& axis, float32 angleRad)
{
    Vector3f n = axis;

    if (n.LengthSquared() <= MX_MATH_EPSILON_SQUARED) return Identity();

    n.Normalize();

    const float32 half = angleRad * 0.5f;
    const float32 s = static_cast<float32>(std::sin(half));
    const float32 c = static_cast<float32>(std::cos(half));

    return Quaternion(n.x * s, n.y * s, n.z * s, c);
}

Quaternion Quaternion::FromYawPitchRollDegrees(float32 yawDeg, float32 pitchDeg, float32 rollDeg)
{
    return FromYawPitchRollRadians(MathUtil::DegToRad(yawDeg), MathUtil::DegToRad(pitchDeg), MathUtil::DegToRad(rollDeg));
}

Quaternion Quaternion::FromYawPitchRollRadians(float32 yawRad, float32 pitchRad, float32 rollRad)
{
    const Quaternion qYaw = FromAxisAngleRadians(Vector3f(0.0f, 1.0f, 0.0f), yawRad);
    const Quaternion qPitch = FromAxisAngleRadians(Vector3f(1.0f, 0.0f, 0.0f), pitchRad);
    const Quaternion qRoll = FromAxisAngleRadians(Vector3f(0.0f, 0.0f, 1.0f), rollRad);

    Quaternion q = Multiply(Multiply(qYaw, qPitch), qRoll);
    q.Normalize();

    return q;
}

Matrix4f Quaternion::ToMatrix(const Quaternion& qIn)
{
    Quaternion q = Normalized(qIn);

    const float32 xx = q.x * q.x;
    const float32 yy = q.y * q.y;
    const float32 zz = q.z * q.z;

    const float32 xy = q.x * q.y;
    const float32 xz = q.x * q.z;
    const float32 yz = q.y * q.z;

    const float32 wx = q.w * q.x;
    const float32 wy = q.w * q.y;
    const float32 wz = q.w * q.z;

    Matrix4f m = Matrix4f::Identity();

    m.m00 = 1.0f - 2.0f * (yy + zz);
    m.m01 = 2.0f * (xy - wz);
    m.m02 = 2.0f * (xz + wy);
    m.m03 = 0.0f;

    m.m10 = 2.0f * (xy + wz);
    m.m11 = 1.0f - 2.0f * (xx + zz);
    m.m12 = 2.0f * (yz - wx);
    m.m13 = 0.0f;

    m.m20 = 2.0f * (xz - wy);
    m.m21 = 2.0f * (yz + wx);
    m.m22 = 1.0f - 2.0f * (xx + yy);
    m.m23 = 0.0f;

    m.m30 = 0.0f;
    m.m31 = 0.0f;
    m.m32 = 0.0f;
    m.m33 = 1.0f;

    return m;
}

Vector3f Quaternion::RotateVector(const Vector3f& v) const
{
    const Quaternion q = Normalized();

    const Vector3f qVector(q.x, q.y, q.z);
    const Vector3f t = 2.0f * Vector3f::Cross(qVector, v);

    return v + q.w * t + Vector3f::Cross(qVector, t);
}

Vector3f Quaternion::RotateVector(const Quaternion& q, const Vector3f& v)
{
    return q.RotateVector(v);
}

Quaternion Quaternion::operator*(const Quaternion& r) const
{
    return Multiply(*this, r);
}

Quaternion& Quaternion::operator*=(const Quaternion& r)
{
    *this = Multiply(*this, r);

    return *this;
}

bool Quaternion::operator==(const Quaternion& r) const
{
    return x == r.x && y == r.y && z == r.z && w == r.w;
}

bool Quaternion::operator!=(const Quaternion& r) const
{
    return !(*this == r);
}
