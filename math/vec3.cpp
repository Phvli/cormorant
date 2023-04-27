#ifdef USE_OPENGL
#   define GLEW_STATIC
#   define GL3_PROTOTYPES 1
#   include <GL/glew.h>
#endif

#include "vec3.h"
#include "util.h"

#include <cmath>     // sqrt
#include <algorithm> // min, max
#include <cstring>   // strlen
#include <cstdlib>   // strtoul

using namespace math;

Vec3::Vec3(const Vec2 &v, float z)
{
    this->x = v.x;
    this->y = v.y;
    this->z = z;
}

Vec3::Vec3(const Vec4 &v)
{
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
}

Vec3::Vec3(const Quat &q)
{
    *this = q.vec3();
}

Vec3::Vec3(const Mat4 &M)
{
    this->x = 0.0f;
    this->y = 0.0f;
    this->z = 0.0f;

    *this *= M;
}

Vec3::Vec3(const char *s)
{
    // remove (potential) leading #
    s += (s[0] == '#');
    
    int   precision = 1 + (strlen(s) > 4);         // 1 for #rgb, 2 for #rrggbb
    float divider   = pow(16.0, precision) - 1.0f; // 15 or 255

    char  component[3];
    float *value[3] = { &this->r, &this->g, &this->b };
    
    for (int i = 0; i < 3; ++i)
    {
        component[0]         = s[0];
        component[1]         = s[1];
        component[precision] = '\0';
        s += precision;
        
        *value[i] = strtoul(component, NULL, 16) / divider;
    }
}

Vec2 Vec3::xy(void) const { return Vec2(this->x, this->y); }
Vec2 Vec3::xz(void) const { return Vec2(this->x, this->z); }
Vec2 Vec3::yz(void) const { return Vec2(this->y, this->z); }

Vec3
Vec3::random(float length)
{
    return Vec3(vary(1.0f), vary(1.0f), vary(1.0f))
        .normalize() * length;
}

float
Vec3::length(void)
const
{
    return sqrt(
        this->x * this->x +
        this->y * this->y +
        this->z * this->z
    );
}

Vec3 &
Vec3::normalize(void)
{
    float d = sqrt(
        this->x * this->x +
        this->y * this->y +
        this->z * this->z
    );
    
    this->x /= d;
    this->y /= d;
    this->z /= d;
    
    return *this;
}

Vec3 &
Vec3::clamp(void)
{
    this->x = std::max(0.0f, std::min(1.0f, this->x));
    this->y = std::max(0.0f, std::min(1.0f, this->y));
    this->z = std::max(0.0f, std::min(1.0f, this->z));

    return *this;
}

Vec3 &
Vec3::clamp(float min, float max)
{
    this->x = std::max(min, std::min(max, this->x));
    this->y = std::max(min, std::min(max, this->y));
    this->z = std::max(min, std::min(max, this->z));

    return *this;
}

Vec3 &
Vec3::saturate(void)
{
    *this -= std::min(std::min(this->x, this->y), this->z);

    float f = std::max(std::max(this->x, this->y), this->z);
    if (f)
    {
        *this *= 1.0f / f;
    }
    else
    {
        this->x = this->y = this->z = 1.0f;
    }

    return *this;
}

Vec3
Vec3::min(const Vec3 &v)
const
{
    return Vec3(
        std::min(this->x, v.x),
        std::min(this->y, v.y),
        std::min(this->z, v.z)
    );
}

Vec3
Vec3::max(const Vec3 &v)
const
{
    return Vec3(
        std::max(this->x, v.x),
        std::max(this->y, v.y),
        std::max(this->z, v.z)
    );
}

Vec3
Vec3::operator*(const Mat4 &M)
const
{
    return Vec3(
        M[ 0] * this->x + M[ 4] * this->y + M[ 8] * this->z + M[12],
        M[ 1] * this->x + M[ 5] * this->y + M[ 9] * this->z + M[13],
        M[ 2] * this->x + M[ 6] * this->y + M[10] * this->z + M[14]
    );
}

Vec3 &
Vec3::operator*=(const Mat4 &M)
{
    float
        x = M[ 0] * this->x + M[ 4] * this->y + M[ 8] * this->z + M[12],
        y = M[ 1] * this->x + M[ 5] * this->y + M[ 9] * this->z + M[13],
        z = M[ 2] * this->x + M[ 6] * this->y + M[10] * this->z + M[14];
    
    this->x = x;
    this->y = y;
    this->z = z;

    return *this;
}

bool
Vec3::is_inside(const Tri3 &triangle)
const
{
    Vec3 p = triangle.barycentric(*this);
    return (p.x >= 0.0f && p.y >= 0.0f && (p.x + p.y) < 1.0f);
}

Vec3
Vec3::rotate(const Quat &q)
{
    Quat rotated
        = q
        * Quat(this->x, this->y, this->z, 0.0f)
        * q.conjugate();
    
    return Vec3(rotated.x, rotated.y, rotated.z);
}

#ifdef USE_OPENGL
Vec3 &
Vec3::to(GLint uniform)
{
    glUniform3f(uniform, this->x, this->y, this->z);

    return *this;
}
#endif
