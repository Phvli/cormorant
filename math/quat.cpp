#include "quat.h"
#include "util.h"

#include <cmath>

using namespace math;

Quat::Quat(const Vec4 &v)
{
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
    this->w = v.w;
}

Quat::Quat(const Mat4& M)
{
    float trace = M.trace();

    if (trace > 0.0f)
    {
        float s = 0.5f / std::sqrt(1.0f + trace);
        this->x = (M[9] - M[6]) * s;
        this->y = (M[2] - M[8]) * s;
        this->z = (M[4] - M[1]) * s;
        this->w = 0.25f / s;
    }
    else if (M[0] > M[5] && M[0] > M[10])
    {
        float s = 2.0f * std::sqrt(1.0f + M[0] - M[5] - M[10]);
        this->x = 0.25f * s;
        this->y = (M[1] + M[4]) / s;
        this->z = (M[2] + M[8]) / s;
        this->w = (M[9] - M[6]) / s;
    }
    else if (M[5] > M[10])
    {
        float s = 2.0f * std::sqrt(1.0f + M[5] - M[0] - M[10]);
        this->x = (M[1] + M[4]) / s;
        this->y = 0.25f * s;
        this->z = (M[6] + M[9]) / s;
        this->w = (M[2] - M[8]) / s;
    }
    else
    {
        float s = 2.0f * std::sqrt(1.0f + M[10] - M[0] - M[5]);
        this->x = (M[2] + M[8]) / s;
        this->y = (M[6] + M[9]) / s;
        this->z = 0.25f * s;
        this->w = (M[4] - M[1]) / s;
    }
    // float trace = a[0][0] + a[1][1] + a[2][2];
    // if (trace > 0.0)
    // {
    //     float s = 0.5f / sqrt(trace + 1.0f);
    //     this->w = 0.25f / s;
    //     this->x = (a[2][1] - a[1][2]) * s;
    //     this->y = (a[0][2] - a[2][0]) * s;
    //     this->z = (a[1][0] - a[0][1]) * s;
    // }
    // else if (a[0][0] > a[1][1] && a[0][0] > a[2][2])
    // {
    //     float s = 2.0f * sqrt(1.0f + a[0][0] - a[1][1] - a[2][2]);
    //     this->w = (a[2][1] - a[1][2]) / s;
    //     this->x = 0.25f * s;
    //     this->y = (a[0][1] + a[1][0]) / s;
    //     this->z = (a[0][2] + a[2][0]) / s;
    // }
    // else if (a[1][1] > a[2][2])
    // {
    //     float s = 2.0f * sqrt(1.0f + a[1][1] - a[0][0] - a[2][2]);
    //     this->w = (a[0][2] - a[2][0]) / s;
    //     this->x = (a[0][1] + a[1][0]) / s;
    //     this->y = 0.25f * s;
    //     this->z = (a[1][2] + a[2][1]) / s;
    // }
    // else
    // {
    //     float s = 2.0f * sqrt(1.0f + a[2][2] - a[0][0] - a[1][1]);
    //     this->w = (a[1][0] - a[0][1]) / s;
    //     this->x = (a[0][2] + a[2][0]) / s;
    //     this->y = (a[1][2] + a[2][1]) / s;
    //     this->z = 0.25f * s;
    // }
}

Quat
Quat::random(void)
{
    return Quat(vary(1.0f), vary(1.0f), vary(1.0f), vary(1.0f))
        .normalize();
}

float
Quat::length(void)
const
{
    return sqrt(
        this->x * this->x +
        this->y * this->y +
        this->z * this->z +
        this->w * this->w
    );
}

Quat &
Quat::normalize(void)
{
    float d = sqrt(
        this->x * this->x +
        this->y * this->y +
        this->z * this->z +
        this->w * this->w
    );
    
    this->x /= d;
    this->y /= d;
    this->z /= d;
    this->w /= d;
    
    return *this;
}

Quat
Quat::operator*(const Quat &q)
const
{
    return Quat(
        this->w * q.x + this->x * q.w + this->y * q.z - this->z * q.y,
        this->w * q.y - this->x * q.z + this->y * q.w + this->z * q.x,
        this->w * q.z + this->x * q.y - this->y * q.x + this->z * q.w,
        this->w * q.w - this->x * q.x - this->y * q.y - this->z * q.z
    );
}

Quat &
Quat::operator*=(const Quat &q)
{
    float
        x   = this->w * q.x + this->x * q.w + this->y * q.z - this->z * q.y,
        y   = this->w * q.y - this->x * q.z + this->y * q.w + this->z * q.x,
        z   = this->w * q.z + this->x * q.y - this->y * q.x + this->z * q.w;
    this->w = this->w * q.w - this->x * q.x - this->y * q.y - this->z * q.z;
    this->x = x;
    this->y = y;
    this->z = z;
    
    return *this;
}

Vec3
Quat::vec3(void)
const
{
    float polarity = this->x * this->y + this->z * this->w;
    
    if (polarity > .499999f)
    {
        // north pole
        return Vec3(
            0.0f,
            std::atan2(this->x, this->w),
            -HALF_PI
        );
    }
    else if (polarity < -.499999f)
    {
        // south pole
        return Vec3(
            0.0f,
            -std::atan2(this->x, this->w),
            HALF_PI
        );
    }
    
    return Vec3(
        std::atan2(2.0f * this->x * this->w - 2.0f * this->y * this->z,
            1.0f - 2.0f * this->x * this->z - 2.0f * this->z * this->z),
        
        std::atan2(2.0f * this->y * this->w - 2.0f * this->x * this->z,
            1.0f - 2.0f * this->y * this->y - 2.0f * this->z * this->z),
        
        -std::asin(2.0f * polarity)
    );
}

Mat4
Quat::mat4(void)
const
{
    return Mat4(
        1.0f - 2.0f * this->y * this->y - 2.0f * this->z * this->z,
        2.0f * this->x * this->y - 2.0f * this->w * this->z,
        2.0f * this->x * this->z + 2.0f * this->w * this->y,
        0.0f,

        2.0f * this->x * this->y + 2.0f * this->w * this->z,
        1.0f - 2.0f * this->x * this->x - 2.0f * this->z * this->z,
        2.0f * this->y * this->z - 2.0f * this->w * this->x,
        0.0f,

        2.0f * this->x * this->z - 2.0f * this->w * this->y,
        2.0f * this->y * this->z + 2.0f * this->w * this->x,
        1.0f - 2.0f * this->x * this->x - 2.0f * this->y * this->y,
        0.0f,

        0.0f,
        0.0f,
        0.0f,
        1.0f
    );
}

Quat
Quat::log(void)
const
{
    float a = acos(this->w);
    float s = sin(a);

    return (s > 0.0f)
        ? Quat(
            a * this->x / s,
            a * this->y / s,
            a * this->z / s,
            0.0f)
        : Quat(0.0f, 0.0f, 0.0f, 0.0f);
}

Quat
Quat::exp()
const
{
    float a = sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
    float s = sin(a);
    float c = cos(a);

    return (a > 0.0f)
        ? Quat(
            s * this->x / a,
            s * this->y / a,
            s * this->z / a,
            c)
        : Quat(0.0f, 0.0f, 0.0f, c);
}

Quat
Quat::rotation(const Vec3 &axis, float angle)
{
    angle  /= 2.0f;
    float s = std::sin(angle);
    return Quat(
        axis.x * s,
        axis.y * s,
        axis.z * s,
        std::cos(angle)
    );
}

Quat
Quat::rotation_x(float angle)
{
    angle /= 2.0f;
    return Quat(
        std::sin(angle),
        0.0f,
        0.0f,
        std::cos(angle)
    );
}

Quat
Quat::rotation_y(float angle)
{
    angle /= 2.0f;
    return Quat(
        0.0f,
        std::sin(angle),
        0.0f,
        std::cos(angle)
    );
}

Quat
Quat::rotation_z(float angle)
{
    angle /= 2.0f;
    return Quat(
        0.0f,
        0.0f,
        std::sin(angle),
        std::cos(angle)
    );
}

Quat
Quat::between(const Vec3 &a, const Vec3 &b)
{
    Vec3 src = Vec3(a).normalize();
    Vec3 dst = Vec3(b).normalize();
    Vec3 rotation_axis;

    float theta = src.dot(dst);

    if (theta < -.9999f)
    {
        // If vectors point in opposite directions,
        // just pick a random rotation axis
        rotation_axis = Vec3(0.0f, 0.0f, 1.0f).cross(src);
        if (rotation_axis.length_sq() < 0.01f)
        {
            // Can't be paraller though, so pick another
            rotation_axis = Vec3(1.0f, 0.0f, 0.0f).cross(src);
        }

        return Quat().rotate(rotation_axis.normalize(), PI);
    }

    float s       = std::sqrt((1.0f + theta) * 2.0f);
    float inv_s   = 1.0f / s;
    rotation_axis = src.cross(dst);

    return Quat(
        rotation_axis.x * inv_s,
        rotation_axis.y * inv_s,
        rotation_axis.z * inv_s,
        s / 2.0f
    );
}

Quat
Quat::facing(const Vec3 &direction)
{
    Vec3 forward = Vec3::back();
    Vec3 toward  = Vec3(direction).normalize();
    float dot = forward.dot(toward);

    if (std::abs(dot + 1.0f) < 0.00001f)
    {
        return Quat::rotation(Vec3(0.0f, 1.0f, 0.0f), PI);
    }
    if (std::abs(dot - 1.0f) < 0.00001f)
    {
        return Quat();
    }

    Vec3 rotation_axis = forward.cross(toward).normalize();
    return Quat::rotation(rotation_axis, std::acos(dot));
}

Quat
Quat::facing(const Vec3 &src, const Vec3 &dst)
{
    return Quat::facing(dst - src);
}

Quat
Quat::slerp(const Quat &q0, const Quat &q1, float ratio)
{
    Quat q;
    float dot = q0.dot(q1);
    
    if (dot < -0.95f || dot > 0.95f)
    {
        return Quat::lerp(q0, q1, ratio);
    }

    float a = acos(dot);
    return (q0 * sin(a * (1.0f - ratio)) + q1 * sin(a * ratio)) / sin(a);
}

Quat
Quat::smooth_slerp(const Quat &q0, const Quat &q1, float ratio)
{
    Quat q;
    float dot = q0.dot(q1);
    
    if (dot < 0.0f)
    {
        // Invert to reduce spinning if the quaternions' diff is over 90 deg
        dot = -dot;
        q   = -q1;
    }
    else
    {
        q = q1;
    }
    
    if (dot > 0.95f)
    {
        return Quat::lerp(q0, q, ratio);
    }

    float a = acos(dot);
    return (q0 * sin(a * (1.0f - ratio)) + q * sin(a * ratio)) / sin(a);
}

Quat
Quat::squad(const Quat &q00, const Quat &q10, const Quat &q01, const Quat &q11, float ratio)
{
    return slerp(
        slerp(q00, q10, ratio),
        slerp(q01, q11, ratio),
        2.0f * ratio * (1.0f - ratio)
    );
}

Quat
Quat::bezier(const Quat &q00, const Quat &q10, const Quat &q01, const Quat &q11, float ratio)
{
    Quat
        a = slerp(q00, q01, ratio),
        b = slerp(q01, q11, ratio),
        c = slerp(q11, q10, ratio);

    return slerp(
        slerp(a, b, ratio),
        slerp(b, c, ratio),
        ratio
    );
}

Quat
Quat::spline(const Quat &q0, const Quat &p, const Quat &q1)
{
    Quat c = p.conjugate();

    return p
        * (
            (
                (c * q0).log() + (c * q1).log()
            ) / -4.0f
        )
        .exp();
}
