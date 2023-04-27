#include "tri3.h"
#include "util.h"

#include <cmath>

using namespace math;

Tri3::Tri3(const Poly3 &p)
{
    this->a = p.v[0];
    this->b = p.v[1];
    this->c = p.v[2];
}

Vec3
Tri3::normal(void)
const
{
    Vec3 u = this->b - this->a;
    Vec3 v = this->c - this->a;

    return Vec3(
        (u.y * v.z) - (u.z * v.y),
        (u.z * v.x) - (u.x * v.z),
        (u.x * v.y) - (u.y * v.x)
    );
}

float
Tri3::norm(void)
const
{
    return sqrt(math::max(
        this->a.length_sq(),
        math::max(
            this->b.length_sq(),
            this->c.length_sq())
    ));
}


Vec3
Tri3::center(void)
const
{
    return Vec3(
        (this->a.x + this->b.x + this->c.x) / 3.0f,
        (this->a.y + this->b.y + this->c.y) / 3.0f,
        (this->a.z + this->b.z + this->c.z) / 3.0f
    );
}

float
Tri3::area(void)
const
{
    float
        x = (this->b.x * this->c.y - this->c.x * this->b.y),
        y = (this->c.x * this->a.y - this->a.x * this->c.y),
        z = (this->a.x * this->b.y - this->b.x * this->a.y);

    return .5f * sqrt(x * x + y * y + z * z);
}

float
Tri3::circumference(void)
const
{
    return (this->a - this->b).length()
        + (this->b - this->c).length()
        + (this->c - this->a).length();
}

Vec3
Tri3::circumsphere(void)
const
{
    Vec3
        ac = this->c - this->a,
        ab = this->b - this->a,
        x  = ab.cross(ac);
    
    return (x.cross(ab) * ac.length_sq()
        + ac.cross(x) * ab.length_sq())
        / (2.0f * x.length_sq());
}

bool
Tri3::intersects(const Vec3 &origin, const Vec3 &direction)
const
{
    Vec3 p = this->barycentric(this->intersection(origin, direction));

    return (p.x >= 0.0f && p.y >= 0.0f && (p.x + p.y) < 1.0f);
}

Vec3
Tri3::intersection(const Vec3 &origin, const Vec3 &direction)
const
{
    Vec3 normal = this->normal();
    Vec3 p      = this->a - origin;
    float t     = normal.dot(p) / normal.dot(direction);

    if (t <= 0.0001)
    {
        return Vec3(-9999.0f, -9999.0f, -9999.0f);
    }

    return origin + direction * t;
}

Vec3
Tri3::barycentric(const Vec3 &p)
const
{
    Vec3 v0 = this->b - this->a;
    Vec3 v1 = this->c - this->a;
    Vec3 v2 = p - this->a;

    float
        d00 = v0.dot(v0),
        d01 = v0.dot(v1),
        d11 = v1.dot(v1),
        d20 = v2.dot(v0),
        d21 = v2.dot(v1);

    float d = d00 * d11 - d01 * d01;

    float
        v = (d11 * d20 - d01 * d21) / d,
        w = (d00 * d21 - d01 * d20) / d;

    return Vec3(v, w, 1.0 - v - w);
}
