// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Core/Math/Matrix.h>
#include <MX/Core/Math/Quaternion.h>
#include <MX/Core/Math/Vector.h>
#include <MX/Core/Math/MathConst.h>
#include <cmath>
#include <utility>

Matrix4f Matrix4f::Identity()
{
    Matrix4f m;
    m.SetIdentity();

    return m;
}

void Matrix4f::SetIdentity()
{
    m00 = 1.0f; m01 = 0.0f; m02 = 0.0f; m03 = 0.0f;
    m10 = 0.0f; m11 = 1.0f; m12 = 0.0f; m13 = 0.0f;
    m20 = 0.0f; m21 = 0.0f; m22 = 1.0f; m23 = 0.0f;
    m30 = 0.0f; m31 = 0.0f; m32 = 0.0f; m33 = 1.0f;
}

void Matrix4f::Transpose()
{
    std::swap(m01, m10);
    std::swap(m02, m20);
    std::swap(m03, m30);
    std::swap(m12, m21);
    std::swap(m13, m31);
    std::swap(m23, m32);
}

Matrix4f Matrix4f::Transposed() const
{
    Matrix4f r = *this;
    r.Transpose();

    return r;
}

Matrix4f Matrix4f::Transposed(const Matrix4f& m)
{
    /*
    Matrix4f r;

    r.m00 = m.m00; r.m01 = m.m10; r.m02 = m.m20; r.m03 = m.m30;
    r.m10 = m.m01; r.m11 = m.m11; r.m12 = m.m21; r.m13 = m.m31;
    r.m20 = m.m02; r.m21 = m.m12; r.m22 = m.m22; r.m23 = m.m32;
    r.m30 = m.m03; r.m31 = m.m13; r.m32 = m.m23; r.m33 = m.m33;
    */

    return m.Transposed();
}

void Matrix4f::Inverse()
{
    const float32 a00 = m00, a01 = m01, a02 = m02, a03 = m03;
    const float32 a10 = m10, a11 = m11, a12 = m12, a13 = m13;
    const float32 a20 = m20, a21 = m21, a22 = m22, a23 = m23;
    const float32 a30 = m30, a31 = m31, a32 = m32, a33 = m33;

    const float32 b00 = a00 * a11 - a01 * a10;
    const float32 b01 = a00 * a12 - a02 * a10;
    const float32 b02 = a00 * a13 - a03 * a10;
    const float32 b03 = a01 * a12 - a02 * a11;
    const float32 b04 = a01 * a13 - a03 * a11;
    const float32 b05 = a02 * a13 - a03 * a12;
    const float32 b06 = a20 * a31 - a21 * a30;
    const float32 b07 = a20 * a32 - a22 * a30;
    const float32 b08 = a20 * a33 - a23 * a30;
    const float32 b09 = a21 * a32 - a22 * a31;
    const float32 b10 = a21 * a33 - a23 * a31;
    const float32 b11 = a22 * a33 - a23 * a32;

    const float32 det =
        b00 * b11 - b01 * b10 + b02 * b09 +
        b03 * b08 - b04 * b07 + b05 * b06;

    if (std::fabs(det) < MX_MATH_EPSILON)
    {
        SetIdentity();
        return;
    }

    const float32 invDet = 1.0f / det;

    Matrix4f r;
    r.m00 = (a11 * b11 - a12 * b10 + a13 * b09) * invDet;
    r.m01 = (-a01 * b11 + a02 * b10 - a03 * b09) * invDet;
    r.m02 = (a31 * b05 - a32 * b04 + a33 * b03) * invDet;
    r.m03 = (-a21 * b05 + a22 * b04 - a23 * b03) * invDet;

    r.m10 = (-a10 * b11 + a12 * b08 - a13 * b07) * invDet;
    r.m11 = (a00 * b11 - a02 * b08 + a03 * b07) * invDet;
    r.m12 = (-a30 * b05 + a32 * b02 - a33 * b01) * invDet;
    r.m13 = (a20 * b05 - a22 * b02 + a23 * b01) * invDet;

    r.m20 = (a10 * b10 - a11 * b08 + a13 * b06) * invDet;
    r.m21 = (-a00 * b10 + a01 * b08 - a03 * b06) * invDet;
    r.m22 = (a30 * b04 - a31 * b02 + a33 * b00) * invDet;
    r.m23 = (-a20 * b04 + a21 * b02 - a23 * b00) * invDet;

    r.m30 = (-a10 * b09 + a11 * b07 - a12 * b06) * invDet;
    r.m31 = (a00 * b09 - a01 * b07 + a02 * b06) * invDet;
    r.m32 = (-a30 * b03 + a31 * b01 - a32 * b00) * invDet;
    r.m33 = (a20 * b03 - a21 * b01 + a22 * b00) * invDet;

    *this = r;
}

Matrix4f Matrix4f::Inversed() const
{
    Matrix4f r = *this;
    r.Inverse();

    return r;
}

Matrix4f Matrix4f::Inversed(const Matrix4f& m)
{
    return m.Inversed();
}

Matrix4f Matrix4f::Mul(const Matrix4f& a, const Matrix4f& b)
{
    Matrix4f r;

    r.m00 = a.m00 * b.m00 + a.m01 * b.m10 + a.m02 * b.m20 + a.m03 * b.m30;
    r.m01 = a.m00 * b.m01 + a.m01 * b.m11 + a.m02 * b.m21 + a.m03 * b.m31;
    r.m02 = a.m00 * b.m02 + a.m01 * b.m12 + a.m02 * b.m22 + a.m03 * b.m32;
    r.m03 = a.m00 * b.m03 + a.m01 * b.m13 + a.m02 * b.m23 + a.m03 * b.m33;

    r.m10 = a.m10 * b.m00 + a.m11 * b.m10 + a.m12 * b.m20 + a.m13 * b.m30;
    r.m11 = a.m10 * b.m01 + a.m11 * b.m11 + a.m12 * b.m21 + a.m13 * b.m31;
    r.m12 = a.m10 * b.m02 + a.m11 * b.m12 + a.m12 * b.m22 + a.m13 * b.m32;
    r.m13 = a.m10 * b.m03 + a.m11 * b.m13 + a.m12 * b.m23 + a.m13 * b.m33;

    r.m20 = a.m20 * b.m00 + a.m21 * b.m10 + a.m22 * b.m20 + a.m23 * b.m30;
    r.m21 = a.m20 * b.m01 + a.m21 * b.m11 + a.m22 * b.m21 + a.m23 * b.m31;
    r.m22 = a.m20 * b.m02 + a.m21 * b.m12 + a.m22 * b.m22 + a.m23 * b.m32;
    r.m23 = a.m20 * b.m03 + a.m21 * b.m13 + a.m22 * b.m23 + a.m23 * b.m33;

    r.m30 = a.m30 * b.m00 + a.m31 * b.m10 + a.m32 * b.m20 + a.m33 * b.m30;
    r.m31 = a.m30 * b.m01 + a.m31 * b.m11 + a.m32 * b.m21 + a.m33 * b.m31;
    r.m32 = a.m30 * b.m02 + a.m31 * b.m12 + a.m32 * b.m22 + a.m33 * b.m32;
    r.m33 = a.m30 * b.m03 + a.m31 * b.m13 + a.m32 * b.m23 + a.m33 * b.m33;

    return r;
}

Matrix4f Matrix4f::FromTranslation(const Vector3f& t)
{
    Matrix4f m = Identity();
    m.m03 = t.x;
    m.m13 = t.y;
    m.m23 = t.z;

    return m;
}

Matrix4f Matrix4f::FromScale(const Vector3f& s)
{
    Matrix4f m = Identity();
    m.m00 = s.x;
    m.m11 = s.y;
    m.m22 = s.z;

    return m;
}

Matrix4f Matrix4f::FromRotation(const Quaternion& q)
{
    return Quaternion::ToMatrix(q);
}

Matrix4f Matrix4f::ViewLookAtRightHanded(const Vector3f& eye, const Vector3f& target, const Vector3f& up)
{
    Vector3f f = target - eye;
    f.Normalize();

    Vector3f s = Vector3f::Cross(f, up);
    s.Normalize();

    Vector3f u = Vector3f::Cross(s, f);

    Matrix4f m = Identity();

    m.m00 = s.x;  m.m01 = s.y;  m.m02 = s.z;  m.m03 = -Vector3f::Dot(s, eye);
    m.m10 = u.x;  m.m11 = u.y;  m.m12 = u.z;  m.m13 = -Vector3f::Dot(u, eye);
    m.m20 = -f.x; m.m21 = -f.y; m.m22 = -f.z; m.m23 = Vector3f::Dot(f, eye);
    m.m30 = 0.0f; m.m31 = 0.0f; m.m32 = 0.0f; m.m33 = 1.0f;

    return m;
}

Matrix4f Matrix4f::ProjectionPerspectiveRightHanded(float32 fovYRad, float32 aspect, float32 zNear, float32 zFar)
{
    const float32 tanHalf = static_cast<float32>(std::tan(fovYRad * 0.5f));

    Matrix4f m{};
    m.m00 = 1.0f / (aspect * tanHalf);
    m.m01 = 0.0f;
    m.m02 = 0.0f;
    m.m03 = 0.0f;

    m.m10 = 0.0f;
    m.m11 = -1.0f / tanHalf;
    m.m12 = 0.0f;
    m.m13 = 0.0f;

    m.m20 = 0.0f;
    m.m21 = 0.0f;
    m.m22 = zFar / (zNear - zFar);
    m.m23 = (zFar * zNear) / (zNear - zFar);

    m.m30 = 0.0f;
    m.m31 = 0.0f;
    m.m32 = -1.0f;
    m.m33 = 0.0f;

    return m;
}

Matrix4f Matrix4f::ProjectionOrthographicRightHanded(float32 width, float32 height, float32 zNear, float32 zFar)
{
    Matrix4f m = Identity();
    m.m00 = 2.0f / width;
    m.m11 = -2.0f / height;
    m.m22 = 1.0f / (zNear - zFar);
    m.m23 = zNear / (zNear - zFar);

    return m;
}

Matrix4f Matrix4f::ViewFromPositionRotation(const Vector3f& position, const Quaternion& rotation)
{
    const Matrix4f r = FromRotation(rotation);
    const Matrix4f rt = Transposed(r);

    const Vector3f negPos = -position;
    const Matrix4f t = FromTranslation(negPos);

    return rt * t;
}

Matrix4f Matrix4f::FromTranslationRotationScale(const Vector3f& position, const Quaternion& rotation, const Vector3f& scale)
{
    return FromTranslation(position) * FromRotation(rotation) * FromScale(scale);
}

Vector4f Matrix4f::operator*(const Vector4f& v) const
{
    return Vector4f(
        m00 * v.x + m01 * v.y + m02 * v.z + m03 * v.w,
        m10 * v.x + m11 * v.y + m12 * v.z + m13 * v.w,
        m20 * v.x + m21 * v.y + m22 * v.z + m23 * v.w,
        m30 * v.x + m31 * v.y + m32 * v.z + m33 * v.w
    );
}

Matrix4f Matrix4f::operator*(const Matrix4f& r) const
{
    return Mul(*this, r);
}

Matrix4f& Matrix4f::operator*=(const Matrix4f& r)
{
    *this = Mul(*this, r);

    return *this;
}
