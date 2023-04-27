#include "poly3.h"
#include "util.h"

#include <cmath>

using namespace math;

Poly3::Poly3(const Poly3 &p)
{
    this->v = p.v;
}

Poly3::Poly3(const Tri3 &p)
{
    this->v.reserve(3);
    this->v.push_back(p.a);
    this->v.push_back(p.b);
    this->v.push_back(p.c);
}

Tri3
Poly3::tri3(void)
const
{
    return Tri3(this->v[0], this->v[1], *(this->v.end() - 1));
}

Poly2
Poly3::xy(void)
const
{
    Poly2 p;
    p.v.reserve(this->v.size());
    
    for (Poly3::Vertices::const_iterator v = this->v.begin();
        v != this->v.end(); ++v)
    {
        p.v.push_back(v->xy());
    }
    
    return p;
}

Poly2
Poly3::xz(void)
const
{
    Poly2 p;
    p.v.reserve(this->v.size());
    
    for (Poly3::Vertices::const_iterator v = this->v.begin();
        v != this->v.end(); ++v)
    {
        p.v.push_back(v->xz());
    }
    
    return p;
}

Poly2
Poly3::yz(void)
const
{
    Poly2 p;
    p.v.reserve(this->v.size());
    
    for (Poly3::Vertices::const_iterator v = this->v.begin();
        v != this->v.end(); ++v)
    {
        p.v.push_back(v->yz());
    }
    
    return p;
}

float
Poly3::norm(void)
const
{
    float f = 0.0f;
    for (Poly3::Vertices::const_iterator v = this->v.begin();
        v != this->v.end(); ++v)
    {
        f = math::max(f, v->length_sq());
    }
    
    return sqrt(f);
}

float
Poly3::circumference(void)
const
{
    float sum = 0.0f;
    
    for (Poly3::Vertices::const_iterator
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

Vec3
Poly3::center(void)
const
{
    Vec3 sum(0.0f);
    float f = 1.0f / (float)this->v.size();
    
    for (Poly3::Vertices::const_iterator v = this->v.begin();
        v != this->v.end(); ++v)
    {
        sum += *v / f;
    }
    
    return sum;
}
