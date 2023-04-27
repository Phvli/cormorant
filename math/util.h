/*
    Collection of miscellaneous
    mathematical utility functions.
    
    Phvli 2017-08-19
*/

#ifndef _MATH_UTIL_H
#define _MATH_UTIL_H

#include "conversions.h"

namespace math
{
    const float
        PI         =   3.1415927f,
        HALF_PI    =   1.5707963f,
        DOUBLE_PI  =   6.2831853f,
        QUARTER_PI =   0.7853982f,

        E          =   2.7182818f;

    float
    rnd(float min, float max);
    // Random real value between min and max
    
    float inline
    rnd(float max) { return rnd(0.0f, max); }
    // Random real value between 0.0f and max
    
    float
    rnd(void);
    // Random real value between 0.0f and 1.0f
    
    bool inline
    probability(float p) { return (rnd() <= p); }
    // Returns true at given probability
    
    float inline
    vary(float max_deviation) { return rnd(-max_deviation, max_deviation); }
    // Real value randomly deviated from 0.0 (+/- max)
    
    float inline
    vary(float baseline, float max_deviation) { return baseline + vary(max_deviation); }
    // Real value randomly deviated from baseline (+/- max)
    
    float
    noise(long n);
    // 1-dimensional noise

    float inline
    noise(int x, int y) { return noise(x + y * 57); }
    // 2-dimensional noise

    float inline
    noise(int x, int y, int z) { return noise(x + y * 57 + z * 2027); }
    // 3-dimensional noise

    float
    perlin_noise(float x, float y, float persistence, int octaves);
    // 2-dimensional Perlin noise
    
    float
    perlin_noise(float x, float y, float z, float persistence, int octaves);
    // 3-dimensional Perlin noise
    
    float inline
    lerp(float v0, float v1, float ratio) { return v0 * (1.0f - ratio) + v1 * ratio; }
    // Linear interpolation between v0 and v1
    
    float inline
    min(float v0, float v1) { bool a = (v0 < v1); return v0 * a + v1 * !a; }
    // Returns the lesser of given values
    
    float inline
    max(float v0, float v1) { bool b = (v0 > v1); return v0 * b + v1 * !b; }
    // Returns the greater of given values
    
    float inline
    clamp(float v, float min, float max) { bool a = (v < min), b = (v > max); return min * a + max * b + v * !(a | b); }
    // Returns v if in given range, otherwise the nearest boundary
    
    int inline
    sgn(float n) { return ((n < 0.0f) * 2 - 1) * (n != 0.0f); }
    // Returns -1 for negative values, +1 for positive and 0 for zero

    float
    deadzone(float value, float size);
    // Rescales value, returns 0.0 in range [-size, size]

    int
    nearest_pow2(int n);
    // Returns the nearest greater or equal power of two

    namespace interpolate
    {
        float inline
        nearest(float v0, float v1, float ratio) { return lerp(v0, v1, (int)(ratio + .5f)); }

        float inline
        linear(float v0, float v1, float ratio) { return lerp(v0, v1, ratio); }

        float
        cosine(float v0, float v1, float ratio);

        float
        coserp(float v0, float v1, float ratio);

        float
        sinerp(float v0, float v1, float ratio);

        float inline
        smoothstep(float v0, float v1, float ratio) { ratio = clamp((ratio - v0) / (v1 - v0), 0.0f, 1.0f); return ratio * ratio * (3.0f - 2.0f * ratio); }
        // Unclamped 3rd order smoothstep (-2x^3 + 3x^2), requires v0 < ratio < v1

        float inline
        smootherstep(float v0, float v1, float ratio) { ratio = clamp((ratio - v0) / (v1 - v0), 0.0f, 1.0f); return ratio * ratio * ratio * (ratio * (ratio * 6.0f - 15.0f) + 10.0f); }
        // Unclamped 5th order smoothstep (6x^5 - 15x^4 + 10x^3), requires v0 < ratio < v1

        float inline
        smootheststep(float v0, float v1, float ratio) { ratio = clamp((ratio - v0) / (v1 - v0), 0.0f, 1.0f); return ratio * ratio * ratio * ratio * (ratio * (ratio * (70.0f - ratio * 20.0f) - 84.0f) + 35.0f); }
        // Unclamped 7th order smoothstep (-20t^7 + 70t^6 - 84t^5 + 35t^4), requires v0 < ratio < v1

        float
        cubic(float p0, float p1, float p2, float p3, float dist);

        float
        catmull_rom(float p0, float p1, float p2, float p3, float dist);

        float
        hermite(float y0, float y1, float y2, float y3, float dist, float tension, float bias);

        float inline
        linear(float x, float x1, float x2, float q00, float q01) { return ((x2 - x) / (x2 - x1)) * q00 + ((x - x1) / (x2 - x1)) * q01; }

        float
        bilinear(float x, float y, float q11, float q12, float q21, float q22, float x1, float x2, float y1, float y2);

        float
        trilinear(float x, float y, float z, float q000, float q001, float q010, float q011, float q100, float q101, float q110, float q111, float x1, float x2, float y1, float y2, float z1, float z2);
    }
    
    namespace transition
    {
        // Interpolation functions whose return value is clamped between [0.0, 1.0]
        float inline nearest(float v0, float v1, float ratio)   { bool a = (ratio < 0.5f); return v0 * a + v1 * !a; }
        float inline linear(float v0, float v1, float ratio)    { return lerp(v0, v1, clamp(ratio, 0.0f, 1.0f)); }
        float inline smooth(float v0, float v1, float ratio)    { ratio = clamp(ratio, 0.0f, 1.0f); return lerp(v0, v1, ratio * ratio * (3.0f - 2.0f * ratio)); }
        float inline smoother(float v0, float v1, float ratio)  { ratio = clamp(ratio, 0.0f, 1.0f); return lerp(v0, v1, ratio * ratio * ratio * (ratio * (ratio * 6.0f - 15.0f) + 10.0f)); }
        float inline smoothest(float v0, float v1, float ratio) { ratio = clamp(ratio, 0.0f, 1.0f); return lerp(v0, v1, ratio * ratio * ratio * ratio * (ratio * (ratio * (70.0f - ratio * 20.0f) - 84.0f) + 35.0f)); }
        float inline ease(float v0, float v1, float ratio)      { return interpolate::cosine(v0, v1, clamp(ratio, 0.0f, 1.0f)); }
        float inline ease_in(float v0, float v1, float ratio)   { return interpolate::coserp(v0, v1, clamp(ratio, 0.0f, 1.0f)); }
        float inline ease_out(float v0, float v1, float ratio)  { return interpolate::sinerp(v0, v1, clamp(ratio, 0.0f, 1.0f)); }
    }
}

#endif
