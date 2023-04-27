#ifndef _MATH_TRIANGLE3_H
#define _MATH_TRIANGLE3_H

#include "vec3.h"

namespace math
{
    class Tri2;
    class Tri3;
    class Poly3;

    class Tri3
    {
        public:
            Vec3 a, b, c;

            Tri3() {}
            Tri3(const Tri2 &t);
            Tri3(const Tri3 &t) { this->a = t.a; this->b = t.b; this->c = t.c; }
            Tri3(const Poly3 &p);
            Tri3(const Vec3 &a, const Vec3 &b, const Vec3 &c) { this->a = a; this->b = b; this->c = c; }

            float
            norm(void) const;
            // Returns the largest vertex length (distance from origin)

            Tri3 &
            normalize(void) { return *this *= 1.0f / this->norm(); }
            // Scales triangle so that all vertices will be in range [-1.0, 1.0]

            Tri3
            normalized(void) const { return *this * (1.0f / this->norm()); }
            // Returns a copy scaled into range [-1.0, 1.0]

            Vec3
            normal(void) const;
            // Calculate a (right-hand) normal vector for this triangle.
            // NOTE: This vector is not normalized and may be of any length.

            Vec3
            center(void) const;
            // Returns the center point.

            float
            area(void) const;
            // Returns the surface area.

            float
            circumference(void) const;
            // Returns the boundary length.

            Vec3
            circumsphere(void) const;
            // Returns a vector which is the offset from the first
            // vector (a) to circumsphere's center point.

            float
            circumradius(void) const { return this->circumsphere().length(); }
            // Returns the radius of the circumsphere.

            Vec3
            circumcenter(void) const { return this->a + this->circumsphere(); }
            // Returns the center point.

            bool
            intersects(const Vec3 &origin, const Vec3 &direction) const;
            // Returns true if the given line intersects this triangle.

            Vec3
            intersection(const Vec3 &origin, const Vec3 &direction) const;
            // Returns the intersection point of
            // - a line (from origin to given direction) and
            // - a plane (formed by this triangle).
            //
            // NOTE: Given point may not be inside the triangle.
            //       Use barycentric() on the return value to identify
            //       whether it lays inside or outside.

            Vec3
            barycentric(const Vec3 &p) const;
            // Calculates the barycentric coordinates of point p.

            Tri3 &
            flip(void) { Vec3 temp = this->b; this->b = this->c; this->c = temp; return *this; }
            // Reverses chirality (= handedness).

            Tri3
            flipped(void) const { return Tri3(this->a, this->c, this->b); }
            // Returns a triangle with reversed chirality.

            bool operator==(const Tri3 &t) const { return (this->a == t.a && this->b == t.b && this->c == t.c); }
            bool operator!=(const Tri3 &t) const { return !(*this == t); }

            Tri3 &operator+=(const Vec3 &v) { this->a += v; this->b += v; this->c += v; return *this; }
            Tri3 &operator-=(const Vec3 &v) { this->a -= v; this->b -= v; this->c -= v; return *this; }
            Tri3 &operator*=(const Vec3 &v) { this->a *= v; this->b *= v; this->c *= v; return *this; }
            Tri3 &operator/=(const Vec3 &v) { this->a /= v; this->b /= v; this->c /= v; return *this; }
            Tri3 &operator+=(float f)       { this->a += f; this->b += f; this->c += f; return *this; }
            Tri3 &operator-=(float f)       { this->a -= f; this->b -= f; this->c -= f; return *this; }
            Tri3 &operator*=(float f)       { this->a *= f; this->b *= f; this->c *= f; return *this; }
            Tri3 &operator/=(float f)       { this->a /= f; this->b /= f; this->c /= f; return *this; }
            
            Tri3 operator+(const Vec3 &v) const { return Tri3(*this) += v; }
            Tri3 operator-(const Vec3 &v) const { return Tri3(*this) -= v; }
            Tri3 operator*(const Vec3 &v) const { return Tri3(*this) *= v; }
            Tri3 operator/(const Vec3 &v) const { return Tri3(*this) /= v; }
            Tri3 operator+(const float f) const { return Tri3(*this) += f; }
            Tri3 operator-(const float f) const { return Tri3(*this) -= f; }
            Tri3 operator*(const float f) const { return Tri3(*this) *= f; }
            Tri3 operator/(const float f) const { return Tri3(*this) /= f; }
    };
}

#include "tri2.h"
#include "poly3.h"

#endif
