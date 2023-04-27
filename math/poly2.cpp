#include "poly2.h"
#include "util.h"

#include <cmath>

using namespace math;

Poly2::Poly2(const Poly2 &p)
{
    this->v = p.v;
}

Poly2::Poly2(const Tri2 &p)
{
    this->v.reserve(3);
    this->v.push_back(p.a);
    this->v.push_back(p.b);
    this->v.push_back(p.c);
}

Tri2
Poly2::tri2(void)
const
{
    return Tri2(this->v[0], this->v[1], *(this->v.end() - 1));
}

float
Poly2::norm(void)
const
{
    float f = 0.0f;
    for (Poly2::Vertices::const_iterator v = this->v.begin();
        v != this->v.end(); ++v)
    {
        f = math::max(f, v->length_sq());
    }
    
    return sqrt(f);
}

float
Poly2::circumference(void)
const
{
    float sum = 0.0f;
    
    for (Poly2::Vertices::const_iterator
        v0 = this->v.begin(),
        v1 = this->v.end() - 1;
        
        v0 != this->v.end();
    )
    {
        sum += (*v0 - *v1).length();
        v1 = v0;
        v0++;
    }

    return sum;
}

Vec2
Poly2::center(void)
const
{
    Vec2 sum(0.0f);
    float f = 1.0f / (float)this->v.size();
    
    for (Poly2::Vertices::const_iterator v = this->v.begin();
        v != this->v.end(); ++v)
    {
        sum += *v / f;
    }
    
    return sum;
}
