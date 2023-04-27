#ifndef _MATH_TRIANGLE2_H
#define _MATH_TRIANGLE2_H

#include "vec2.h"

namespace math
{
    class Tri2;
    class Poly2;

    class Tri2
    {
        public:
            Vec2 a, b, c;

            Tri2() {}
            Tri2(const Tri2 &t) { this->a = t.a; this->b = t.b; this->c = t.c; }
            Tri2(const Tri3 &t);
            Tri2(const Poly2 &p);
            Tri2(const Vec2 &a, const Vec2 &b, const Vec2 &c) { this->a = a; this->b = b; this->c = c; }

            float
            norm(void) const;
            // Returns the largest vertex length (distance from origin)

            Tri2 &
            normalize(void) { return *this *= 1.0f / this->norm(); }
            // Scales triangle so that all vertices will be in range [-1.0, 1.0]

            Tri2
            normalized(void) const { return *this * (1.0f / this->norm()); }
            // Returns a copy scaled into range [-1.0, 1.0]

            Vec2
            center(void) const;
            // Returns the center point.

            float
            area(void) const;
            // Returns the surface area.

            float
            circumference(void) const;
            // Returns the boundary length.

            Vec2
            circumcircle(void) const;
            // Returns a vector which is the offset from the first
            // vector (a) to circumcircles center point.

            float
            circumradius(void) const { return this->circumcircle().length(); }
            // Returns the radius of the circumcircle.

            bool
            intersects(const Vec2 &origin, const Vec2 &direction) const;
            // Returns true if the given line intersects this triangle.

            Tri2 &
            flip(void) { Vec2 temp = this->b; this->b = this->c; this->c = temp; return *this; }
            // Reverses chirality (= handedness).

            Tri2
            flipped(void) const { return Tri2(this->a, this->c, this->b); }
            // Returns a triangle with reversed chirality.

            Vec3
            barycentric(const Vec2 &p) const;
            // Calculates the barycentric coordinates of point p.

            bool operator==(const Tri2 &t) const { return (this->a == t.a && this->b == t.b && this->c == t.c); }
            bool operator!=(const Tri2 &t) const { return !(*this == t); }

            Tri2 &operator+=(const Vec2 &v) { this->a += v; this->b += v; this->c += v; return *this; }
            Tri2 &operator-=(const Vec2 &v) { this->a -= v; this->b -= v; this->c -= v; return *this; }
            Tri2 &operator*=(const Vec2 &v) { this->a *= v; this->b *= v; this->c *= v; return *this; }
            Tri2 &operator/=(const Vec2 &v) { this->a /= v; this->b /= v; this->c /= v; return *this; }
            Tri2 &operator+=(float f)       { this->a += f; this->b += f; this->c += f; return *this; }
            Tri2 &operator-=(float f)       { this->a -= f; this->b -= f; this->c -= f; return *this; }
            Tri2 &operator*=(float f)       { this->a *= f; this->b *= f; this->c *= f; return *this; }
            Tri2 &operator/=(float f)       { this->a /= f; this->b /= f; this->c /= f; return *this; }
            
            Tri2 operator+(const Vec2 &v) const { return Tri2(*this) += v; }
            Tri2 operator-(const Vec2 &v) const { return Tri2(*this) -= v; }
            Tri2 operator*(const Vec2 &v) const { return Tri2(*this) *= v; }
            Tri2 operator/(const Vec2 &v) const { return Tri2(*this) /= v; }
            Tri2 operator+(const float f) const { return Tri2(*this) += f; }
            Tri2 operator-(const float f) const { return Tri2(*this) -= f; }
            Tri2 operator*(const float f) const { return Tri2(*this) *= f; }
            Tri2 operator/(const float f) const { return Tri2(*this) /= f; }
    };
}

#include "poly2.h"

#endif
