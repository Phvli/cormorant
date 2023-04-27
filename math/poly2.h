#ifndef _MATH_POLYGON2_H
#define _MATH_POLYGON2_H

#include "vec2.h"
#include "tri2.h"
#include <vector>

namespace math
{
    class Poly2;

    class Poly2
    {
        public:
            typedef
                std::vector<Vec2>
                Vertices;
            
            typedef
                std::vector<Tri2>
                Triangles;
            
            Vertices v;

            Poly2() {}
            Poly2(const Tri2 &t);
            Poly2(const Poly2 &p);
            Poly2(const Vertices &list) { this->v = list; }
            Poly2(const Vec2 &v) { this->v.push_back(v); }
            Poly2(const Vec2 &v0, const Vec2 &v1) { this->v.reserve(2); this->v.push_back(v0); this->v.push_back(v1); }
            Poly2(const Vec2 &v0, const Vec2 &v1, const Vec2 &v2) { this->v.reserve(3); this->v.push_back(v0); this->v.push_back(v1); this->v.push_back(v2); }
            Poly2(const Vec2 &v0, const Vec2 &v1, const Vec2 &v2, const Vec2 &v3) { this->v.reserve(4); this->v.push_back(v0); this->v.push_back(v1); this->v.push_back(v2); this->v.push_back(v3); }
            Poly2(const Vec2 &v0, const Vec2 &v1, const Vec2 &v2, const Vec2 &v3, const Vec2 &v4) { this->v.reserve(5); this->v.push_back(v0); this->v.push_back(v1); this->v.push_back(v2); this->v.push_back(v3); this->v.push_back(v4); }
            Poly2(const Vec2 &v0, const Vec2 &v1, const Vec2 &v2, const Vec2 &v3, const Vec2 &v4, const Vec2 &v5) { this->v.reserve(6); this->v.push_back(v0); this->v.push_back(v1); this->v.push_back(v2); this->v.push_back(v3); this->v.push_back(v4); this->v.push_back(v5); }
            Poly2(const Vec2 &v0, const Vec2 &v1, const Vec2 &v2, const Vec2 &v3, const Vec2 &v4, const Vec2 &v5, const Vec2 &v6) { this->v.reserve(7); this->v.push_back(v0); this->v.push_back(v1); this->v.push_back(v2); this->v.push_back(v3); this->v.push_back(v4); this->v.push_back(v5); this->v.push_back(v6); }
            Poly2(const Vec2 &v0, const Vec2 &v1, const Vec2 &v2, const Vec2 &v3, const Vec2 &v4, const Vec2 &v5, const Vec2 &v6, const Vec2 &v7) { this->v.reserve(8); this->v.push_back(v0); this->v.push_back(v1); this->v.push_back(v2); this->v.push_back(v3); this->v.push_back(v4); this->v.push_back(v5); this->v.push_back(v6); this->v.push_back(v7); }
            Poly2(const Vec2 &v0, const Vec2 &v1, const Vec2 &v2, const Vec2 &v3, const Vec2 &v4, const Vec2 &v5, const Vec2 &v6, const Vec2 &v7, const Vec2 &v8) { this->v.reserve(9); this->v.push_back(v0); this->v.push_back(v1); this->v.push_back(v2); this->v.push_back(v3); this->v.push_back(v4); this->v.push_back(v5); this->v.push_back(v6); this->v.push_back(v7); this->v.push_back(v8); }
            Poly2(const Vec2 &v0, const Vec2 &v1, const Vec2 &v2, const Vec2 &v3, const Vec2 &v4, const Vec2 &v5, const Vec2 &v6, const Vec2 &v7, const Vec2 &v8, const Vec2 &v9) { this->v.reserve(10); this->v.push_back(v0); this->v.push_back(v1); this->v.push_back(v2); this->v.push_back(v3); this->v.push_back(v4); this->v.push_back(v5); this->v.push_back(v6); this->v.push_back(v7); this->v.push_back(v8); this->v.push_back(v9); }

            Tri2
            tri2(void) const;
            // Returns a triangle formed by the first, second and last vertices.
            // Undefined for a polygon containing less than 3 vertices.

            float
            norm(void) const;
            // Returns the largest vertex lenght (distance from origin)

            Poly2 &
            normalize(void) { return *this *= 1.0f / this->norm(); }
            // Scales polygon so that all vertices will be in range [-1.0, 1.0]

            Poly2
            normalized(void) const { return *this * (1.0f / this->norm()); }
            // Returns a copy scaled into range [-1.0, 1.0]

            Vec2
            center(void) const;
            // Returns average position of vertices.

            float
            area(void) const;
            // Returns the surface area.

            float
            circumference(void) const;
            // Returns boundary length.

            float
            circumradius(void) const;
            // Returns the radius of the circumtriangle.

            Vec2
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

            bool operator==(const Poly2 &p) const { if (p.v.size() != this->v.size()) return false; for (Vertices::const_iterator v0 = this->v.begin(), v1 = p.v.begin(); v0 != this->v.end(); ++v0, ++v1) if (*v0 != *v1) return false; return true; }
            bool operator!=(const Poly2 &p) const { return !(*this == p); }

            Poly2 &operator+=(const Vec2 &v) { for (Vertices::iterator i = this->v.begin(); i != this->v.end(); (*i++) += v); return *this; }
            Poly2 &operator-=(const Vec2 &v) { for (Vertices::iterator i = this->v.begin(); i != this->v.end(); (*i++) -= v); return *this; }
            Poly2 &operator*=(const Vec2 &v) { for (Vertices::iterator i = this->v.begin(); i != this->v.end(); (*i++) *= v); return *this; }
            Poly2 &operator/=(const Vec2 &v) { for (Vertices::iterator i = this->v.begin(); i != this->v.end(); (*i++) /= v); return *this; }
            Poly2 &operator+=(float f)       { for (Vertices::iterator i = this->v.begin(); i != this->v.end(); (*i++) += f); return *this; }
            Poly2 &operator-=(float f)       { for (Vertices::iterator i = this->v.begin(); i != this->v.end(); (*i++) -= f); return *this; }
            Poly2 &operator*=(float f)       { for (Vertices::iterator i = this->v.begin(); i != this->v.end(); (*i++) *= f); return *this; }
            Poly2 &operator/=(float f)       { for (Vertices::iterator i = this->v.begin(); i != this->v.end(); (*i++) /= f); return *this; }
            
            Poly2 operator+(const Vec2 &v) const { return Poly2(*this) += v; }
            Poly2 operator-(const Vec2 &v) const { return Poly2(*this) -= v; }
            Poly2 operator*(const Vec2 &v) const { return Poly2(*this) *= v; }
            Poly2 operator/(const Vec2 &v) const { return Poly2(*this) /= v; }
            Poly2 operator+(const float f) const { return Poly2(*this) += f; }
            Poly2 operator-(const float f) const { return Poly2(*this) -= f; }
            Poly2 operator*(const float f) const { return Poly2(*this) *= f; }
            Poly2 operator/(const float f) const { return Poly2(*this) /= f; }
    };
}

#endif
