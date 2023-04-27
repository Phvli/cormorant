#ifdef USE_OPENGL
#   define GLEW_STATIC
#   define GL3_PROTOTYPES 1
#   include <GL/glew.h>
#endif

#include "vec4.h"
#include "util.h"

#include <cmath>     // sqrt
#include <algorithm> // min, max
#include <cstring>   // strlen
#include <cstdlib>   // strtoul

using namespace math;

Vec4::Vec4(const Vec2 &v, float z, float w)
{
    this->x = v.x;
    this->y = v.y;
    this->z = z;
    this->w = w;
}

Vec4::Vec4(const Vec3 &v, float z)
{
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
    this->w = w;
}

Vec4::Vec4(const Mat4 &M)
{
    this->x = 0.0f;
    this->y = 0.0f;
    this->z = 0.0f;
    this->w = 0.0f;

    *this *= M;
}

Vec4::Vec4(const Quat &q)
{
    this->x = q.x;
    this->y = q.y;
    this->z = q.z;
    this->w = q.w;
}

Vec4::Vec4(const char *s)
{
    // remove (potential) leading #
    s += (s[0] == '#');
    
    int   precision = 1 + (strlen(s) > 6);         // 1 for #rgba, 2 for #rrggbbaa
    float divider   = pow(16.0, precision) - 1.0f; // 15 or 255

    char  component[4];
    float *value[4] = { &this->r, &this->g, &this->b, &this->a };
    
    for (int i = 0; i < 3; ++i)
    {
        component[0]         = s[0];
        component[1]         = s[1];
        component[2]         = s[2];
        component[precision] = '\0';
        s += precision;
        
        *value[i] = strtoul(component, NULL, 16) / divider;
    }
}

Vec3 Vec4::xyz(void) const { return Vec3(this->x, this->y, this->z); }
Vec3 Vec4::xyw(void) const { return Vec3(this->x, this->y, this->w); }
Vec3 Vec4::xzw(void) const { return Vec3(this->x, this->z, this->w); }
Vec3 Vec4::yzw(void) const { return Vec3(this->y, this->z, this->w); }

Vec2 Vec4::xy(void)  const { return Vec2(this->x, this->y); }
Vec2 Vec4::xz(void)  const { return Vec2(this->x, this->z); }
Vec2 Vec4::xw(void)  const { return Vec2(this->x, this->w); }
Vec2 Vec4::yz(void)  const { return Vec2(this->y, this->z); }
Vec2 Vec4::yw(void)  const { return Vec2(this->y, this->w); }

Vec4
Vec4::random(float length)
{
    return Vec4(vary(1.0f), vary(1.0f), vary(1.0f), vary(1.0f))
        .normalize() * length;
}

float
Vec4::length(void)
const
{
    return sqrt(
        this->x * this->x +
        this->y * this->y +
        this->z * this->z +
        this->w * this->w
    );
}

Vec4 &
Vec4::normalize(void)
{
    float d = sqrt(
        this->x * this->x +
        this->y * this->y +
        this->z * this->z +
        this->w * this->w
    );
    
    this->x /= d;
    this->y /= d;
    this->z /= d;
    this->w /= d;
    
    return *this;
}

Vec4 &
Vec4::clamp(void)
{
    this->x = std::max(0.0f, std::min(1.0f, this->x));
    this->y = std::max(0.0f, std::min(1.0f, this->y));
    this->z = std::max(0.0f, std::min(1.0f, this->z));
    this->w = std::max(0.0f, std::min(1.0f, this->w));

    return *this;
}

Vec4 &
Vec4::clamp(float min, float max)
{
    this->x = std::max(min, std::min(max, this->x));
    this->y = std::max(min, std::min(max, this->y));
    this->z = std::max(min, std::min(max, this->z));
    this->w = std::max(min, std::min(max, this->w));

    return *this;
}

Vec4 &
Vec4::saturate(void)
{
    *this -= std::min(std::min(std::min(this->x, this->y), this->z), this->w);

    float f = std::max(std::max(std::max(this->x, this->y), this->z), this->w);
    if (f)
    {
        *this *= 1.0f / f;
    }
    else
    {
        this->x = this->y = this->z = this->w = 1.0f;
    }

    return *this;
}

Vec4
Vec4::min(const Vec4 &v)
const
{
    return Vec4(
        std::min(this->x, v.x),
        std::min(this->y, v.y),
        std::min(this->z, v.z),
        std::min(this->w, v.w)
    );
}

Vec4
Vec4::max(const Vec4 &v)
const
{
    return Vec4(
        std::max(this->x, v.x),
        std::max(this->y, v.y),
        std::max(this->z, v.z),
        std::max(this->w, v.w)
    );
}

Vec4
Vec4::operator*(const Mat4 &M)
const
{
    return Vec4(
        M[ 0] * this->x + M[ 4] * this->y + M[ 8] * this->z + M[12] * this->w,
        M[ 1] * this->x + M[ 5] * this->y + M[ 9] * this->z + M[13] * this->w,
        M[ 2] * this->x + M[ 6] * this->y + M[10] * this->z + M[14] * this->w,
        M[ 3] * this->x + M[ 7] * this->y + M[11] * this->z + M[15] * this->w
    );
}

Vec4 &
Vec4::operator*=(const Mat4 &M)
{
    float
        x = M[ 0] * this->x + M[ 4] * this->y + M[ 8] * this->z + M[12] * this->w,
        y = M[ 1] * this->x + M[ 5] * this->y + M[ 9] * this->z + M[13] * this->w,
        z = M[ 2] * this->x + M[ 6] * this->y + M[10] * this->z + M[14] * this->w,
        w = M[ 3] * this->x + M[ 7] * this->y + M[11] * this->z + M[15] * this->w;
    
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;

    return *this;
}

#ifdef USE_OPENGL
Vec4 &
Vec4::to(GLint uniform)
{
    glUniform4f(uniform, this->x, this->y, this->z, this->w);

    return *this;
}
#endif
