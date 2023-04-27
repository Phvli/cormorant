#include "line2.h"

#include <cmath>

using namespace math;

float
Line2::length(void)
const
{
    return (a - b).length();
}

float
Line2::length_sq(void)
const
{
    return (a - b).length_sq();
}

Vec2
Line2::direction(void)
const
{
    return (b - a).normal();
}

bool
Line2::intersects(const Line2 &l)
const
{
    Vec2 r = this->b - this->a;
    Vec2 s = l.b     - l.a;
    Vec2 q = l.a     - this->a;

    float t = q.cross(s) / r.cross(s);
    float u = q.cross(r) / r.cross(s);

    return
        (t > 0.0 && t <= 1.0) &&
        (u > 0.0 && u <= 1.0);
}

Vec2
Line2::get_intersection(const Line2 &l)
const
{
    float intersection_x =
          (this->a.x * this->b.y - this->a.y * this->b.x) * (l.a.x - l.b.x)
        - (l.a.x * l.b.y - l.a.y * l.b.x) * (this->a.x - this->b.x);

    float intersection_y =
          (this->a.x * this->b.y - this->a.y * this->b.x) * (l.a.y - l.b.y)
        - (l.a.x * l.b.y - l.a.y * l.b.x) * (this->a.y - this->b.y);

    float d =
          (this->a.x - this->b.x) * (l.a.y - l.b.y)
        - (this->a.y - this->b.y) * (l.a.x - l.b.x);

    return Vec2(intersection_x / d, intersection_y / d);
}
