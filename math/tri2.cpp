#include "tri2.h"
#include "util.h"

#include <cmath>

using namespace math;

Tri2::Tri2(const Poly2 &p)
{
    this->a = p.v[0];
    this->b = p.v[1];
    this->c = p.v[2];
}

float
Tri2::norm(void)
const
{
    return sqrt(math::max(
        this->a.length_sq(),
        math::max(
            this->b.length_sq(),
            this->c.length_sq())
    ));
}


Vec2
Tri2::center(void)
const
{
    return Vec2(
        (this->a.x + this->b.x + this->c.x) / 3.0f,
        (this->a.y + this->b.y + this->c.y) / 3.0f
    );
}

float
Tri2::area(void)
const
{
    float
        x = (this->b.x * this->c.y - this->c.x * this->b.y),
        y = (this->c.x * this->a.y - this->a.x * this->c.y),
        z = (this->a.x * this->b.y - this->b.x * this->a.y);

    return .5f * sqrt(x * x + y * y + z * z);
}

float
Tri2::circumference(void)
const
{
    return (this->a - this->b).length()
        + (this->b - this->c).length()
        + (this->c - this->a).length();
}

Vec2
Tri2::circumcircle(void)
const
{
    Vec2
        v,
        c = this->center();
    
    float
        ca = (c - this->a).length_sq(),
        cb = (c - this->b).length_sq(),
        cc = (c - this->c).length_sq();
    
    if (ca > cb)
    {
        v = (ca > cc) ? this->a : this->c;
    }
    else
    {
        v = (cb > cc) ? this->b : this->c;
    }

    return c - v;
}

Vec3
Tri2::barycentric(const Vec2 &p)
const
{
    Vec2 v0 = this->b - this->a;
    Vec2 v1 = this->c - this->a;
    Vec2 v2 = p - this->a;

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
