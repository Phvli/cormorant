#ifndef _MATH_POLYGON3_H
#define _MATH_POLYGON3_H

#include "vec3.h"
#include "tri3.h"
#include "poly2.h"
#include <vector>

namespace math
{
    class Poly3;

    class Poly3
    {
        public:
            typedef
                std::vector<Vec3>
                Vertices;
            
            typedef
                std::vector<Tri3>
                Triangles;
            
            Vertices v;

            Poly3() {}
            Poly3(const Tri3 &t);
            Poly3(const Poly3 &p);
            Poly3(const Vertices &list) { this->v = list; }
            Poly3(const Vec3 &v) { this->v.push_back(v); }
            Poly3(const Vec3 &v0, const Vec3 &v1) { this->v.reserve(2); this->v.push_back(v0); this->v.push_back(v1); }
            Poly3(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2) { this->v.reserve(3); this->v.push_back(v0); this->v.push_back(v1); this->v.push_back(v2); }
            Poly3(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, const Vec3 &v3) { this->v.reserve(4); this->v.push_back(v0); this->v.push_back(v1); this->v.push_back(v2); this->v.push_back(v3); }
            Poly3(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, const Vec3 &v3, const Vec3 &v4) { this->v.reserve(5); this->v.push_back(v0); this->v.push_back(v1); this->v.push_back(v2); this->v.push_back(v3); this->v.push_back(v4); }
            Poly3(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, const Vec3 &v3, const Vec3 &v4, const Vec3 &v5) { this->v.reserve(6); this->v.push_back(v0); this->v.push_back(v1); this->v.push_back(v2); this->v.push_back(v3); this->v.push_back(v4); this->v.push_back(v5); }
            Poly3(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, const Vec3 &v3, const Vec3 &v4, const Vec3 &v5, const Vec3 &v6) { this->v.reserve(7); this->v.push_back(v0); this->v.push_back(v1); this->v.push_back(v2); this->v.push_back(v3); this->v.push_back(v4); this->v.push_back(v5); this->v.push_back(v6); }
            Poly3(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, const Vec3 &v3, const Vec3 &v4, const Vec3 &v5, const Vec3 &v6, const Vec3 &v7) { this->v.reserve(8); this->v.push_back(v0); this->v.push_back(v1); this->v.push_back(v2); this->v.push_back(v3); this->v.push_back(v4); this->v.push_back(v5); this->v.push_back(v6); this->v.push_back(v7); }
            Poly3(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, const Vec3 &v3, const Vec3 &v4, const Vec3 &v5, const Vec3 &v6, const Vec3 &v7, const Vec3 &v8) { this->v.reserve(9); this->v.push_back(v0); this->v.push_back(v1); this->v.push_back(v2); this->v.push_back(v3); this->v.push_back(v4); this->v.push_back(v5); this->v.push_back(v6); this->v.push_back(v7); this->v.push_back(v8); }
            Poly3(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, const Vec3 &v3, const Vec3 &v4, const Vec3 &v5, const Vec3 &v6, const Vec3 &v7, const Vec3 &v8, const Vec3 &v9) { this->v.reserve(10); this->v.push_back(v0); this->v.push_back(v1); this->v.push_back(v2); this->v.push_back(v3); this->v.push_back(v4); this->v.push_back(v5); this->v.push_back(v6); this->v.push_back(v7); this->v.push_back(v8); this->v.push_back(v9); }

            Tri3
            tri3(void) const;
            // Returns a triangle formed by the first, second and last vertices.
            // Undefined for a polygon containing less than 3 vertices.

            Poly2 xy(void) const;
            Poly2 xz(void) const;
            Poly2 yz(void) const;

            float
            norm(void) const;
            // Returns the largest vertex lenght (distance from origin)

            Poly3 &
            normalize(void) { return *this *= 1.0f / this->norm(); }
            // Scales polygon so that all vertices will be in range [-1.0, 1.0]

            Poly3
            normalized(void) const { return *this * (1.0f / this->norm()); }
            // Returns a copy scaled into range [-1.0, 1.0]

            Vec3
            normal(void) const { return this->tri3().normal(); }
            // Calculate a (right-hand) normal vector for this polygon.
            // Assumes the surface to be completely flat.
            // Undefined for a polygon containing less than 3 vertices.
            // NOTE: This vector is not normalized and may be of any lenght.

            Vec3
            center(void) const;
            // Returns average position of vertices.

            float
            area(void) const;
            // Returns the surface area.
            // Assumes the surface to be completely flat.

            float
            circumference(void) const;
            // Returns boundary length.

            float
            circumradius(void) const;
            // Returns the radius of the circumtriangle.

            Vec3
            circumcenter(void) const;
            // Returns the center point.

            void
            subdiv(int iterations = 1, bool linear = true);
            // Splits all edges in two

            void
            contract(int iterations = 1);
            // Reverse subdivide, halves vertex count
            
            Triangles
            triangulate(void) const;

            bool operator==(const Poly3 &p) const { if (p.v.size() != this->v.size()) return false; for (Vertices::const_iterator v0 = this->v.begin(), v1 = p.v.begin(); v0 != this->v.end(); ++v0, ++v1) if (*v0 != *v1) return false; return true; }
            bool operator!=(const Poly3 &p) const { return !(*this == p); }

            Poly3 &operator+=(const Vec3 &v) { for (Vertices::iterator i = this->v.begin(); i != this->v.end(); (*i++) += v); return *this; }
            Poly3 &operator-=(const Vec3 &v) { for (Vertices::iterator i = this->v.begin(); i != this->v.end(); (*i++) -= v); return *this; }
            Poly3 &operator*=(const Vec3 &v) { for (Vertices::iterator i = this->v.begin(); i != this->v.end(); (*i++) *= v); return *this; }
            Poly3 &operator/=(const Vec3 &v) { for (Vertices::iterator i = this->v.begin(); i != this->v.end(); (*i++) /= v); return *this; }
            Poly3 &operator+=(float f)       { for (Vertices::iterator i = this->v.begin(); i != this->v.end(); (*i++) += f); return *this; }
            Poly3 &operator-=(float f)       { for (Vertices::iterator i = this->v.begin(); i != this->v.end(); (*i++) -= f); return *this; }
            Poly3 &operator*=(float f)       { for (Vertices::iterator i = this->v.begin(); i != this->v.end(); (*i++) *= f); return *this; }
            Poly3 &operator/=(float f)       { for (Vertices::iterator i = this->v.begin(); i != this->v.end(); (*i++) /= f); return *this; }
            
            Poly3 operator+(const Vec3 &v) const { return Poly3(*this) += v; }
            Poly3 operator-(const Vec3 &v) const { return Poly3(*this) -= v; }
            Poly3 operator*(const Vec3 &v) const { return Poly3(*this) *= v; }
            Poly3 operator/(const Vec3 &v) const { return Poly3(*this) /= v; }
            Poly3 operator+(const float f) const { return Poly3(*this) += f; }
            Poly3 operator-(const float f) const { return Poly3(*this) -= f; }
            Poly3 operator*(const float f) const { return Poly3(*this) *= f; }
            Poly3 operator/(const float f) const { return Poly3(*this) /= f; }
    };
}

#endif
