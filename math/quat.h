#ifndef _MATH_QUAT_H
#define _MATH_QUAT_H

namespace math
{

    class Vec3;
    class Vec4;
    class Mat4;
    class Quat;
    class Quat
    {
        public:
            float x, y, z, w;

            Quat(): x(0.0f), y(0.0f), z(0.0f), w(1.0f) {};
            Quat(const Quat &q) { this->x = q.x; this->y = q.y; this->z = q.z; this->w = q.w; }
            Quat(const Vec3 &v); // from euler angles
            Quat(const Vec4 &v);
            Quat(const Mat4 &M);
            Quat(float f) { this->x = f; this->y = f; this->z = f; this->w = 1.0f; }
            Quat(float x, float y, float z = 0.0f, float w = 1.0f) { this->x = x; this->y = y; this->z = z; this->w = w; }
            static Quat random(void);

            float
            length(void) const;
            // Return vector length of this quaternion

            Quat &
            normalize(void);
            // Normalize so that lenght will equal to 1.0f

            float
            norm(void) const { return x * x + y * y + z * z + w * w; }
            
            Quat
            conjugate(void) const { return Quat(-x, -y, -z, w); }
            
            Quat
            inverse(void) const { return this->conjugate() * 1.0f / this->norm(); }

            Quat &
            invert(void) { return *this = this->inverse(); }

            Vec3
            vec3(void) const;
            // Get angular rotation for each axis as a Vec3

            Mat4
            mat4(void) const;
            // Get rotation matrix

            float
            dot(const Quat &q) const { return x * q.x + y * q.y + z * q.z + w * q.w; }
            // Dot product between 2 quaternions
            
            Quat log(void) const;
            Quat exp(void) const;

            // Rotations
            static Quat rotation(const Vec3 &axis, float angle);
            static Quat rotation_x(float angle);
            static Quat rotation_y(float angle);
            static Quat rotation_z(float angle);

            Quat &rotate(const Vec3 &axis, float angle) { return *this *= Quat::rotation(axis, angle); }
            Quat &rotate_x(float angle) { return *this *= Quat::rotation_x(angle); }
            Quat &rotate_y(float angle) { return *this *= Quat::rotation_y(angle); }
            Quat &rotate_z(float angle) { return *this *= Quat::rotation_z(angle); }
            // Rotate around given axis

            static Quat
            between(const Vec3 &a, const Vec3 &b);
            // Rotation between vectors as a quaternion

            static Quat
            facing(const Vec3 &direction);
            // Rotated towards given direction
            
            static Quat
            facing(const Vec3 &src, const Vec3 &dst);
            // Direction from src to dst

            // Interpolation
            // (linear, spherical, cubic spherical, Shoemaker-Bezier and spline respectively)
            static Quat lerp(const Quat &q0, const Quat &q1, float ratio) { return (q0 * (1.0f - ratio) + q1 * ratio).normalize(); }
            static Quat slerp(const Quat &q0, const Quat &q1, float ratio);
            static Quat smooth_slerp(const Quat &q0, const Quat &q1, float ratio); // Reduces spinning if the quaternions are over 90 deg apart
            static Quat squad(const Quat &q00, const Quat &q10, const Quat &q01, const Quat &q11, float ratio);
            static Quat bezier(const Quat &q00, const Quat &q10, const Quat &q01, const Quat &q11, float ratio);
            static Quat spline(const Quat &q0, const Quat &p, const Quat &q1);
            Quat &lerp(const Quat &q, float ratio) { return *this = Quat::lerp(*this, q, ratio); }
            Quat &slerp(const Quat &q, float ratio) { return *this = Quat::slerp(*this, q, ratio); }
            Quat &smooth_slerp(const Quat &q, float ratio) { return *this = Quat::smooth_slerp(*this, q, ratio); }

            // Quaternion multiplication
            Quat &operator*=(const Quat &q);
            Quat  operator*(const Quat &q) const;

            // Comparisons compare the end result of a rotation with a small epsilon threshold
            bool operator==(const Quat &q) const { float diff = this->dot(q); return (diff > -.00001f && diff < .00001f); }
            bool operator!=(const Quat &q) const { return !(*this == q); }
            
            // Negation
            Quat operator-() const { return this->inverse(); }

            // Other operations
            Quat &operator+=(const Quat &q) { x += q.x; y += q.y; z += q.z; w += q.w; return *this; }
            Quat &operator-=(const Quat &q) { x -= q.x; y -= q.y; z -= q.z; w -= q.w; return *this; }
            Quat &operator/=(const Quat &q) { return *this *= q.inverse(); }
            Quat &operator+=(float f)       { x += f;   y += f;   z += f;   w += f;   return *this; }
            Quat &operator-=(float f)       { x -= f;   y -= f;   z -= f;   w -= f;   return *this; }
            Quat &operator*=(float f)       { x *= f;   y *= f;   z *= f;   w *= f;   return *this; }
            Quat &operator/=(float f)       { x /= f;   y /= f;   z /= f;   w /= f;   return *this; }

            Quat operator+(const Quat &q) const { return Quat(*this) += q; }
            Quat operator-(const Quat &q) const { return Quat(*this) -= q; }
            Quat operator/(const Quat &q) const { return *this * q.inverse(); }
            Quat operator+(const float f) const { return Quat(*this) += f; }
            Quat operator-(const float f) const { return Quat(*this) -= f; }
            Quat operator*(const float f) const { return Quat(*this) *= f; }
            Quat operator/(const float f) const { return Quat(*this) /= f; }
    };
}

#include "vec3.h"
#include "vec4.h"
#include "mat4.h"

#endif
