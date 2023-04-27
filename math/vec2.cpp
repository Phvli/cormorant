#ifdef USE_OPENGL
#   define GLEW_STATIC
#   define GL3_PROTOTYPES 1
#   include <GL/glew.h>
#endif

#include "vec2.h"
#include "util.h"

#include <cmath>
#include <algorithm>

using namespace math;

Vec2::Vec2(const Vec3 &v)
{
    this->x = v.x;
    this->y = v.y;
}


Vec2::Vec2(const Vec4 &v)
{
    this->x = v.x;
    this->y = v.y;
}

Vec2
Vec2::random(float length)
{
    return Vec2(vary(1.0f), vary(1.0f))
        .normalize() * length;
}

float
Vec2::length(void)
const
{
    return sqrt(
        this->x * this->x +
        this->y * this->y
    );
}

Vec2 &
Vec2::normalize(void)
{
    float d = sqrt(
        this->x * this->x +
        this->y * this->y
    );
    
    this->x /= d;
    this->y /= d;
    
    return *this;
}

Vec2 &
Vec2::clamp(void)
{
    this->x = std::max(0.0f, std::min(1.0f, this->x));
    this->y = std::max(0.0f, std::min(1.0f, this->y));

    return *this;
}

Vec2 &
Vec2::clamp(float min, float max)
{
    this->x = std::max(min, std::min(max, this->x));
    this->y = std::max(min, std::min(max, this->y));

    return *this;
}

Vec2 &
Vec2::saturate(void)
{
    *this -= std::min(this->x, this->y);

    float f = std::max(this->x, this->y);
    if (f)
    {
        *this *= 1.0f / f;
    }
    else
    {
        this->x = this->y = 1.0f;
    }

    return *this;
}

Vec2
Vec2::min(const Vec2 &v)
const
{
    return Vec2(
        std::min(this->x, v.x),
        std::min(this->y, v.y)
    );
}

Vec2
Vec2::max(const Vec2 &v)
const
{
    return Vec2(
        std::max(this->x, v.x),
        std::max(this->y, v.y)
    );
}

Vec2
Vec2::operator*(const Mat4 &M)
const
{
    return Vec2(
        M[ 0] * this->x + M[ 4] * this->y + M[12],
        M[ 1] * this->x + M[ 5] * this->y + M[13]
    );
}

Vec2 &
Vec2::operator*=(const Mat4 &M)
{
    float
        x = M[ 0] * this->x + M[ 4] * this->y + M[ 12],
        y = M[ 1] * this->x + M[ 5] * this->y + M[ 13];
    
    this->x = x;
    this->y = y;

    return *this;
}

bool
Vec2::is_inside(const Tri2 &triangle)
const
{
    Vec3 p = triangle.barycentric(*this);
    return (p.x >= 0.0f && p.y >= 0.0f && (p.x + p.y) < 1.0f);
}

#ifdef USE_OPENGL
Vec2 &
Vec2::to(GLint uniform)
{
    glUniform2f(uniform, this->x, this->y);

    return *this;
}
#endif
