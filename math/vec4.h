#ifndef _MATH_VECTOR_4_H
#define _MATH_VECTOR_4_H

#ifdef USE_OPENGL
#   include <GL/gl.h>
#endif

#include "../gfx/sprite.h"

namespace math
{
    class Vec2;
    class Vec3;
    class Vec4;
    class Quat;
    class Mat4;
    class Tri3;

    class Vec4
    {
        public:
            union { float x, r; };
            union { float y, g; };
            union { float z, b; };
            union { float w, a; };

            // Unit vectors
            static Vec4 X(void)    { return Vec4(1.0f, 0.0f, 0.0f, 0.0f); }
            static Vec4 Y(void)    { return Vec4(0.0f, 1.0f, 0.0f, 0.0f); }
            static Vec4 Z(void)    { return Vec4(0.0f, 0.0f, 1.0f, 0.0f); }
            static Vec4 W(void)    { return Vec4(0.0f, 0.0f, 0.0f, 1.0f); }
            static Vec4 zero(void) { return Vec4(0.0f, 0.0f, 0.0f, 0.0f); }
            static Vec4 random(float length = 1.0f);
            
            Vec4(): x(0.0f), y(0.0f), z(0.0f), w(0.0f) {};
            Vec4(const Vec2 &v, float z = 0.0f, float w = 0.0f); // From 2D vector
            Vec4(const Vec3 &v, float w = 0.0f); // From 3D vector
            Vec4(const Quat &q);
            Vec4(const Mat4 &M); // From 4 x 4 matrix
            Vec4(const char *s); // From HTML color code ("RGBA", "#RGBA", "RRGGBBAA" or "#RRGGBBAA")
            Vec4(float f) { this->x = f; this->y = f; this->z = f; this->w = f; }
            Vec4(float x, float y, float z = 0.0f, float w = 0.0f) { this->x = x; this->y = y; this->z = z; this->w = w; }
            Vec4(gfx::Color argb) { a = (float)((argb >> 24) & 0xff) / 256.0f; r = (float)((argb >> 16) & 0xff) / 256.0f; g = (float)((argb >> 8) & 0xff) / 256.0f; b = (float)(argb & 0xff) / 256.0f; }

            Vec3 xyz(void) const;
            Vec3 xyw(void) const;
            Vec3 xzw(void) const;
            Vec3 yzw(void) const;
            Vec2 xy(void) const;
            Vec2 xz(void) const;
            Vec2 xw(void) const;
            Vec2 yz(void) const;
            Vec2 yw(void) const;

            float
            length(void) const;

            float
            length_sq(void) const { return x * x + y * y + z * z + w * w; }

            float
            dist(Vec4 &v) const { return Vec4(*this - v).length(); };

            float
            dist_sq(Vec4 &v) const { return Vec4(*this - v).length_sq(); };

            Vec4 &
            normalize(void);

            Vec4
            normal(void) const { return Vec4(*this).normalize(); }

            float
            dot(const Vec4 &v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }

            Vec4
            min(const Vec4 &v) const;

            Vec4
            max(const Vec4 &v) const;

            Vec4
            mix(const Vec4 &v, float ratio) const { float i_ratio = 1.0f - ratio; return Vec4(x * i_ratio + v.x * ratio, y * i_ratio + v.y * ratio, z * i_ratio + v.z * ratio, w * i_ratio + v.w * ratio); }

            static Vec4
            interpolate(const Vec4 &v0, const Vec4 &v1, float ratio) { return v0.mix(v1, ratio); }

            Vec4 &operator+=(const Vec4 &v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
            Vec4 &operator-=(const Vec4 &v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
            Vec4 &operator*=(const Vec4 &v) { x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
            Vec4 &operator/=(const Vec4 &v) { x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this; }
            Vec4 &operator+=(float f)       { x += f;   y += f;   z += f;   w += f;   return *this; }
            Vec4 &operator-=(float f)       { x -= f;   y -= f;   z -= f;   w -= f;   return *this; }
            Vec4 &operator*=(float f)       { x *= f;   y *= f;   z *= f;   w *= f;   return *this; }
            Vec4 &operator/=(float f)       { x /= f;   y /= f;   z /= f;   w /= f;   return *this; }

            Vec4 operator+(const Vec4 &v) const { return Vec4(*this) += v; }
            Vec4 operator-(const Vec4 &v) const { return Vec4(*this) -= v; }
            Vec4 operator*(const Vec4 &v) const { return Vec4(*this) *= v; }
            Vec4 operator/(const Vec4 &v) const { return Vec4(*this) /= v; }
            Vec4 operator+(const float f) const { return Vec4(*this) += f; }
            Vec4 operator-(const float f) const { return Vec4(*this) -= f; }
            Vec4 operator*(const float f) const { return Vec4(*this) *= f; }
            Vec4 operator/(const float f) const { return Vec4(*this) /= f; }
            

            Vec4 operator*(const Mat4 &M) const;
            Vec4 &operator*=(const Mat4 &M);

            Vec4 operator-() const { return Vec4(-x, -y, -z, -w); }
            
            // Comparisons
            bool operator==(const Vec4 &v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
            bool operator!=(const Vec4 &v) const { return !(*this == v); }
            bool operator> (const Vec4 &v) const { return (x * x + y * y + z * z + w * w) >  (v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w); }
            bool operator>=(const Vec4 &v) const { return (x * x + y * y + z * z + w * w) >= (v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w); }
            bool operator< (const Vec4 &v) const { return (x * x + y * y + z * z + w * w) <  (v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w); }
            bool operator<=(const Vec4 &v) const { return (x * x + y * y + z * z + w * w) <= (v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w); }
            
            // Range limiting
            Vec4 &saturate(void);              // scales all components between 0.0 and 1.0
            Vec4 &clamp(float min, float max); // clamps all components without preserving ratios
            Vec4 &clamp(void);                 // does a clamp between 0.0 and 1.0

#   ifdef USE_OPENGL
            Vec4 &to(GLint uniform);
#   endif
    };
}

#include "vec2.h"
#include "vec3.h"
#include "quat.h"
#include "mat4.h"
#include "tri3.h"

#endif
