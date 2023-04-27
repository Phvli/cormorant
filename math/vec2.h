#ifndef _MATH_VECTOR_2_H
#define _MATH_VECTOR_2_H

#ifdef USE_OPENGL
#   include <GL/gl.h>
#endif

namespace math
{
    class Vec2;
    class Vec3;
    class Vec4;
    class Tri2;
    class Tri3;
    class Mat4;

    class Vec2
    {
        public:
            float x;
            union { float y, z; };

            // Unit vectors
            static Vec2 X(void)     { return Vec2(+1.0f,  0.0f); }
            static Vec2 Y(void)     { return Vec2( 0.0f, +1.0f); }
            static Vec2 up(void)    { return Vec2( 0.0f, +1.0f); }
            static Vec2 down(void)  { return Vec2( 0.0f, -1.0f); }
            static Vec2 left(void)  { return Vec2(-1.0f,  1.0f); }
            static Vec2 right(void) { return Vec2(+1.0f,  1.0f); }
            static Vec2 zero(void)  { return Vec2( 0.0f,  0.0f); }
            static Vec2 random(float length = 1.0f);
            
            Vec2(): x(0.0f), y(0.0f) {};
            Vec2(const Vec3 &v); // From 3D vector
            Vec2(const Vec4 &v); // From 4D vector
            Vec2(float f) { this->x = f; this->y = f; }
            Vec2(float x, float y) { this->x = x; this->y = y; }

            float
            length(void) const;

            float
            length_sq(void) const { return x * x + y * y; }

            float
            dist(Vec2 &v) const { return Vec2(*this - v).length(); };

            float
            dist_sq(Vec2 &v) const { return Vec2(*this - v).length_sq(); };

            Vec2 &
            normalize(void);

            Vec2
            normal(void) const { return Vec2(*this).normalize(); }

            float
            dot(const Vec2 &v) const { return x * v.x + y * v.y; }

            float
            cross(const Vec2 &v) const { return x * v.y - y * v.x; }

            float
            cross2D(const Vec2 &v) const { return x * v.y - y * v.x; }

            Vec2
            min(const Vec2 &v) const;

            Vec2
            max(const Vec2 &v) const;

            Vec2
            mix(const Vec2 &v, float ratio) const { float i_ratio = 1.0f - ratio; return Vec2(x * i_ratio + v.x * ratio, y * i_ratio + v.y * ratio); }

            static Vec2
            interpolate(const Vec2 &v0, const Vec2 &v1, float ratio) { return v0.mix(v1, ratio); }

            Vec2 &operator+=(const Vec2 &v) { x += v.x; y += v.y; return *this; }
            Vec2 &operator-=(const Vec2 &v) { x -= v.x; y -= v.y; return *this; }
            Vec2 &operator*=(const Vec2 &v) { x *= v.x; y *= v.y; return *this; }
            Vec2 &operator/=(const Vec2 &v) { x /= v.x; y /= v.y; return *this; }
            Vec2 &operator+=(float f)       { x += f;   y += f;   return *this; }
            Vec2 &operator-=(float f)       { x -= f;   y -= f;   return *this; }
            Vec2 &operator*=(float f)       { x *= f;   y *= f;   return *this; }
            Vec2 &operator/=(float f)       { x /= f;   y /= f;   return *this; }

            Vec2 operator+(const Vec2 &v) const { return Vec2(*this) += v; }
            Vec2 operator-(const Vec2 &v) const { return Vec2(*this) -= v; }
            Vec2 operator*(const Vec2 &v) const { return Vec2(*this) *= v; }
            Vec2 operator/(const Vec2 &v) const { return Vec2(*this) /= v; }
            Vec2 operator+(const float f) const { return Vec2(*this) += f; }
            Vec2 operator-(const float f) const { return Vec2(*this) -= f; }
            Vec2 operator*(const float f) const { return Vec2(*this) *= f; }
            Vec2 operator/(const float f) const { return Vec2(*this) /= f; }

            Vec2 operator*(const Mat4 &M) const;
            Vec2 &operator*=(const Mat4 &M);

            Vec2 operator-() const { return Vec2(-x, -y); }
            
            // Comparisons
            bool operator==(const Vec2 &v) const { return x == v.x && y == v.y; }
            bool operator!=(const Vec2 &v) const { return !(*this == v); }
            bool operator> (const Vec2 &v) const { return (x * x + y * y) >  (v.x * v.x + v.y * v.y); }
            bool operator>=(const Vec2 &v) const { return (x * x + y * y) >= (v.x * v.x + v.y * v.y); }
            bool operator< (const Vec2 &v) const { return (x * x + y * y) <  (v.x * v.x + v.y * v.y); }
            bool operator<=(const Vec2 &v) const { return (x * x + y * y) <= (v.x * v.x + v.y * v.y); }

            // Range limiting
            Vec2 &saturate(void);              // scales all components between 0.0 and 1.0
            Vec2 &clamp(float min, float max); // clamps all components without preserving ratios
            Vec2 &clamp(void);                 // does a clamp between 0.0 and 1.0

            bool
            is_inside(const Tri2 &triangle) const;

#   ifdef USE_OPENGL
            Vec2 &to(GLint uniform);
#   endif
    };
}

#include "vec3.h"
#include "vec4.h"
#include "tri2.h"
#include "tri3.h"
#include "mat4.h"

#endif
