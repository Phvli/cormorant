/* TODO
    a)
        float -> GLfloat
    
    b)
        float v[3];
        float &operator[] (int i) { return v[i]; }
        float &x() { return v[0]; }
        float &y() { return v[1]; }
        float &z() { return v[2]; }
        float &r() { return v[0]; }
        float &g() { return v[1]; }
        float &b() { return v[2]; }
*/

#ifndef _MATH_VECTOR_3_H
#define _MATH_VECTOR_3_H

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

    class Vec3
    {
        public:
            union { float x, r; };
            union { float y, g; };
            union { float z, b; };

            // Unit vectors
            static Vec3 X(void)       { return Vec3(+1.0f,  0.0f,  0.0f); }
            static Vec3 Y(void)       { return Vec3( 0.0f, +1.0f,  0.0f); }
            static Vec3 Z(void)       { return Vec3( 0.0f,  0.0f, +1.0f); }
            static Vec3 up(void)      { return Vec3( 0.0f, +1.0f,  0.0f); }
            static Vec3 down(void)    { return Vec3( 0.0f, -1.0f,  0.0f); }
            static Vec3 left(void)    { return Vec3(-1.0f,  1.0f,  0.0f); }
            static Vec3 right(void)   { return Vec3(+1.0f,  1.0f,  0.0f); }
            static Vec3 forward(void) { return Vec3( 0.0f,  0.0f, +1.0f); }
            static Vec3 back(void)    { return Vec3( 0.0f,  0.0f, -1.0f); }
            static Vec3 zero(void)    { return Vec3( 0.0f,  0.0f,  0.0f); }
            static Vec3 random(float length = 1.0f);
            
            Vec3(): x(0.0f), y(0.0f), z(0.0f) {};
            Vec3(const Vec2 &v, float z = 0.0f); // From 2D vector
            Vec3(const Vec4 &v); // From 4D vector
            Vec3(const Quat &q); // Rotation axes from quaternion
            Vec3(const Mat4 &M); // From 4 x 4 matrix
            Vec3(const char *s); // From HTML color code ("RGB", "#RGB", "RRGGBB" or "#RRGGBB")
            Vec3(float f) { this->x = f; this->y = f; this->z = f; }
            Vec3(float x, float y, float z = 0.0f) { this->x = x; this->y = y; this->z = z; }
            Vec3(gfx::Color rgb) { r = (float)((rgb >> 16) & 0xff) / 256.0f; g = (float)((rgb >> 8) & 0xff) / 256.0f; b = (float)(rgb & 0xff) / 256.0f; }

            Vec2 xy(void) const;
            Vec2 xz(void) const;
            Vec2 yz(void) const;

            float
            length(void) const;

            float
            length_sq(void) const { return x * x + y * y + z * z; }

            float
            dist(Vec3 &v) const { return Vec3(*this - v).length(); };

            float
            dist_sq(Vec3 &v) const { return Vec3(*this - v).length_sq(); };

            Vec3 &
            normalize(void);

            Vec3
            normal(void) const { return Vec3(*this).normalize(); }

            float
            dot(const Vec3 &v) const { return x * v.x + y * v.y + z * v.z; }

            Vec3
            cross(const Vec3 &v) const { return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }

            float
            cross2D(const Vec3 &v) const { return x * v.y - y * v.x; }
            
            Vec3
            min(const Vec3 &v) const;

            Vec3
            max(const Vec3 &v) const;

            Vec3
            mix(const Vec3 &v, float ratio) const { float i_ratio = 1.0f - ratio; return Vec3(x * i_ratio + v.x * ratio, y * i_ratio + v.y * ratio, z * i_ratio + v.z * ratio); }

            static Vec3
            interpolate(const Vec3 &v0, const Vec3 &v1, float ratio) { return v0.mix(v1, ratio); }

            Vec3 &operator+=(const Vec3 &v) { x += v.x; y += v.y; z += v.z; return *this; }
            Vec3 &operator-=(const Vec3 &v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
            Vec3 &operator*=(const Vec3 &v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
            Vec3 &operator/=(const Vec3 &v) { x /= v.x; y /= v.y; z /= v.z; return *this; }
            Vec3 &operator+=(float f)       { x += f;   y += f;   z += f;   return *this; }
            Vec3 &operator-=(float f)       { x -= f;   y -= f;   z -= f;   return *this; }
            Vec3 &operator*=(float f)       { x *= f;   y *= f;   z *= f;   return *this; }
            Vec3 &operator/=(float f)       { x /= f;   y /= f;   z /= f;   return *this; }

            Vec3 operator+(const Vec3 &v) const { return Vec3(*this) += v; }
            Vec3 operator-(const Vec3 &v) const { return Vec3(*this) -= v; }
            Vec3 operator*(const Vec3 &v) const { return Vec3(*this) *= v; }
            Vec3 operator/(const Vec3 &v) const { return Vec3(*this) /= v; }
            Vec3 operator+(const float f) const { return Vec3(*this) += f; }
            Vec3 operator-(const float f) const { return Vec3(*this) -= f; }
            Vec3 operator*(const float f) const { return Vec3(*this) *= f; }
            Vec3 operator/(const float f) const { return Vec3(*this) /= f; }
            

            Vec3 operator*(const Mat4 &M) const;
            Vec3 &operator*=(const Mat4 &M);

            Vec3 operator-() const { return Vec3(-x, -y, -z); }
            
            // Comparisons
            bool operator==(const Vec3 &v) const { return x == v.x && y == v.y && z == v.z; }
            bool operator!=(const Vec3 &v) const { return !(*this == v); }
            bool operator> (const Vec3 &v) const { return (x * x + y * y + z * z) >  (v.x * v.x + v.y * v.y + v.z * v.z); }
            bool operator>=(const Vec3 &v) const { return (x * x + y * y + z * z) >= (v.x * v.x + v.y * v.y + v.z * v.z); }
            bool operator< (const Vec3 &v) const { return (x * x + y * y + z * z) <  (v.x * v.x + v.y * v.y + v.z * v.z); }
            bool operator<=(const Vec3 &v) const { return (x * x + y * y + z * z) <= (v.x * v.x + v.y * v.y + v.z * v.z); }
            
            // Range limiting
            Vec3 &saturate(void);              // scales all components between 0.0 and 1.0
            Vec3 &clamp(float min, float max); // clamps all components without preserving ratios
            Vec3 &clamp(void);                 // does a clamp between 0.0 and 1.0

            bool
            is_inside(const Tri3 &triangle) const;
            
            Vec3
            rotate(const Quat &q);

#   ifdef USE_OPENGL
            Vec3 &to(GLint uniform);
#   endif
    };
}

#include "vec2.h"
#include "vec4.h"
#include "quat.h"
#include "mat4.h"
#include "tri3.h"

#endif
