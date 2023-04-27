#include "util.h"

#include <cmath>   // cos
#include <cstdlib> // rand

static float
    noisebuf[64],
    buffered_2D_noise(int i),
    buffered_3D_noise(int i);

float
math::noise(long n)
{
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

float
math::rnd(void)
{
    return (float)rand() / RAND_MAX;
}

float
math::rnd(float min, float max)
{
    return min + (max - min) * (float)rand() / RAND_MAX;
}

float
math::interpolate::cosine(float a, float b, float ratio)
{
    ratio = (1.0f - cos(ratio * math::PI)) * .5f;
    return a * (1.0f - ratio) + b * ratio;
}

float
math::interpolate::coserp(float a, float b, float ratio)
{
    ratio = (1.0f - cos(ratio * math::HALF_PI));
    return a * (1.0f - ratio) + b * ratio;
}

float
math::interpolate::sinerp(float a, float b, float ratio)
{
    ratio = sin(ratio * math::HALF_PI);
    return a * (1.0f - ratio) + b * ratio;
}

float
math::interpolate::cubic(float p0, float p1, float p2, float p3, float dist)
{
    float dist_sq = dist * dist;
    float p = (p3 - p2) - (p0 - p1);
    float q = (p0 - p1) - p;
    float r = p2 - p0;
    
    return p * dist_sq * dist
        + q * dist_sq
        + r * dist
        + p0;
}

float
math::interpolate::catmull_rom(float p0, float p1, float p2, float p3, float dist)
{
    float dist_sq = dist * dist;
    float p = -0.5f * p0 + 1.5f * p1 - 1.5f * p2 + 0.5f * p3;
    float q =         p0 - 2.5f * p1 + 2.0f * p2 - 0.5f * p3;
    float r = -0.5f * p0 + 0.5f * p2;
    
    return p * dist_sq * dist
        + q * dist_sq
        + r * dist
        + p0;
}

float
math::interpolate::hermite(float y0, float y1, float y2, float y3, float dist, float tension, float bias)
{
    float m0, m1, dist2, dist3;
    float a0, a1, a2, a3;

    dist2 = dist * dist;
    dist3 = dist2 * dist;
    m0    = (y1 - y0) * (1.0f + bias) * (1.0f - tension) / 2.0f;
    m0   += (y2 - y1) * (1.0f - bias) * (1.0f - tension) / 2.0f;
    m1    = (y2 - y1) * (1.0f + bias) * (1.0f - tension) / 2.0f;
    m1   += (y3 - y2) * (1.0f - bias) * (1.0f - tension) / 2.0f;
    a0    = 2.0f * dist3 - 3.0f * dist2 + 1.0f;
    a1    = dist3 - 2.0f * dist2 + dist;
    a2    = dist3 - dist2;
    a3    = -2.0f * dist3 + 3.0f * dist2;

    return(a0 * y1 + a1 * m0 + a2 * m1 + a3 * y2);
}

float
math::interpolate::bilinear(float x, float y, float q11, float q12, float q21, float q22, float x1, float x2, float y1, float y2)
{
    return math::interpolate::linear(y, y1, y2,
        math::interpolate::linear(x, x1, x2, q11, q21),
        math::interpolate::linear(x, x1, x2, q12, q22)
    );
}

float
math::interpolate::trilinear(float x, float y, float z, float q000, float q001, float q010, float q011, float q100, float q101, float q110, float q111, float x1, float x2, float y1, float y2, float z1, float z2)
{
    return math::interpolate::linear(z, z1, z2,
        math::interpolate::linear(y, y1, y2,
            math::interpolate::linear(x, x1, x2, q000, q100),
            math::interpolate::linear(x, x1, x2, q001, q101)
        ),
        math::interpolate::linear(y, y1, y2,
            math::interpolate::linear(x, x1, x2, q010, q110),
            math::interpolate::linear(x, x1, x2, q011, q111)
        )
    );
}

float
math::perlin_noise(float x, float y, float persistence, int octaves)
{
    float buf_x, buf_y;
    
    float total = 0;
    
    for (int i = 0; i < octaves; ++i) {
        float frequency = pow(2, i);
        float amplitude = pow(persistence, i);
        
        buf_x = x * frequency;
        buf_y = y * frequency;
        
        // precalc noise
        noisebuf[ 0] = noise(buf_x - 1, buf_y - 1);
        noisebuf[ 1] = noise(buf_x    , buf_y - 1);
        noisebuf[ 2] = noise(buf_x + 1, buf_y - 1);
        noisebuf[ 3] = noise(buf_x + 2, buf_y - 1);
        noisebuf[ 4] = noise(buf_x - 1, buf_y    );
        noisebuf[ 5] = noise(buf_x    , buf_y    );
        noisebuf[ 6] = noise(buf_x + 1, buf_y    );
        noisebuf[ 7] = noise(buf_x + 2, buf_y    );
        noisebuf[ 8] = noise(buf_x - 1, buf_y + 1);
        noisebuf[ 9] = noise(buf_x    , buf_y + 1);
        noisebuf[10] = noise(buf_x + 1, buf_y + 1);
        noisebuf[11] = noise(buf_x + 2, buf_y + 1);
        noisebuf[12] = noise(buf_x - 1, buf_y + 2);
        noisebuf[13] = noise(buf_x    , buf_y + 2);
        noisebuf[14] = noise(buf_x + 1, buf_y + 2);
        noisebuf[15] = noise(buf_x + 2, buf_y + 2);
        
        buf_x -= (int)buf_x;
        buf_y -= (int)buf_y;
        
        float upper = math::interpolate::cosine(
            buffered_2D_noise(5),  // (x + 0, y + 0)
            buffered_2D_noise(6),  // (x + 1, y + 0)
            buf_x);
        
        float lower = math::interpolate::cosine(
            buffered_2D_noise(9),  // (x + 0, y + 1)
            buffered_2D_noise(10), // (x + 1, y + 1)
            buf_x);
        
        total += math::interpolate::cosine(upper, lower, buf_y) * amplitude;
    }
    
    return total;
}

float
math::perlin_noise(float x, float y, float z, float persistence, int octaves)
{
    float buf_x, buf_y, buf_z;
    float *buf_index;
    
    float total = 0;
    
    for (int i = 0; i < octaves; ++i) {
        float frequency = pow(2, i);
        float amplitude = pow(persistence, i);
        
        buf_y = y * frequency;
        buf_x = x * frequency;
        buf_z = z * frequency - 1;
        
        // precalc noise
        buf_index = noisebuf;
        for (int n = -1; n <= 2; ++n) {
            *(buf_index++) = noise(buf_x - 1, buf_y - 1, buf_z);
            *(buf_index++) = noise(buf_x    , buf_y - 1, buf_z);
            *(buf_index++) = noise(buf_x + 1, buf_y - 1, buf_z);
            *(buf_index++) = noise(buf_x + 2, buf_y - 1, buf_z);
            *(buf_index++) = noise(buf_x - 1, buf_y    , buf_z);
            *(buf_index++) = noise(buf_x    , buf_y    , buf_z);
            *(buf_index++) = noise(buf_x + 1, buf_y    , buf_z);
            *(buf_index++) = noise(buf_x + 2, buf_y    , buf_z);
            *(buf_index++) = noise(buf_x - 1, buf_y + 1, buf_z);
            *(buf_index++) = noise(buf_x    , buf_y + 1, buf_z);
            *(buf_index++) = noise(buf_x + 1, buf_y + 1, buf_z);
            *(buf_index++) = noise(buf_x + 2, buf_y + 1, buf_z);
            *(buf_index++) = noise(buf_x - 1, buf_y + 2, buf_z);
            *(buf_index++) = noise(buf_x    , buf_y + 2, buf_z);
            *(buf_index++) = noise(buf_x + 1, buf_y + 2, buf_z);
            *(buf_index++) = noise(buf_x + 2, buf_y + 2, buf_z);
            buf_z++;
        }
        
        buf_x -= (int)buf_x;
        buf_y -= (int)buf_y;
        buf_z -= (int)buf_z + 4;
        
        float upper = math::interpolate::cosine(
            buffered_3D_noise(21),  // (x + 0, y + 0, z + 0)
            buffered_3D_noise(22),  // (x + 1, y + 0, z + 0)
            buf_x);
        
        float lower = math::interpolate::cosine(
            buffered_3D_noise(25),  // (x + 0, y + 1, z + 0)
            buffered_3D_noise(26),  // (x + 1, y + 1, z + 0)
            buf_x);
        
        float closer = math::interpolate::cosine(upper, lower, buf_y) * amplitude;
        
        upper = math::interpolate::cosine(
            buffered_3D_noise(37),  // (x + 0, y + 0, z + 1)
            buffered_3D_noise(38),  // (x + 1, y + 0, z + 1)
            buf_x);
        
        lower = math::interpolate::cosine(
            buffered_3D_noise(41),  // (x + 0, y + 1, z + 1)
            buffered_3D_noise(42),  // (x + 1, y + 1, z + 1)
            buf_x);
        
        float farther = math::interpolate::cosine(upper, lower, buf_y) * amplitude;
        
        total += math::interpolate::cosine(closer, farther, buf_z) * amplitude;
    }
    
    return total;
}

static float
buffered_2D_noise(int i)
{
    return
        // corners
           (noisebuf[i - 5] + noisebuf[i - 3] + noisebuf[i + 3] + noisebuf[i + 5]) / 16.0f
        // sides
         + (noisebuf[i - 4] + noisebuf[i - 1] + noisebuf[i + 1] + noisebuf[i + 4]) / 8.0f
        // middle
         +  noisebuf[i] / 4.0f;
}

static float
buffered_3D_noise(int i)
{
    // (!) FIXME: TODO: group together
    return
        // upper layer - corners
           (noisebuf[i - 21] + noisebuf[i - 19] + noisebuf[i - 13] + noisebuf[i - 11]) / 64.0f
        // upper layer - sides
         + (noisebuf[i - 20] + noisebuf[i - 17] + noisebuf[i - 15] + noisebuf[i - 12]) / 32.0f
        // upper layer - middle
         +  noisebuf[i - 16] / 16.0f
        
        // middle layer - corners
         + (noisebuf[i -  5] + noisebuf[i -  3] + noisebuf[i +  3] + noisebuf[i +  5]) / 32.0f
        // middle layer - sides
         + (noisebuf[i -  4] + noisebuf[i -  1] + noisebuf[i +  1] + noisebuf[i +  4]) / 16.0f
        // middle layer - middle
         +  noisebuf[i] / 8.0f
        
        // lower layer - corners
         + (noisebuf[i + 11] + noisebuf[i + 13] + noisebuf[i + 19] + noisebuf[i + 21]) / 64.0f
        // lower layer - sides
         + (noisebuf[i + 12] + noisebuf[i + 15] + noisebuf[i + 17] + noisebuf[i + 20]) / 32.0f
        // lower layer - middle
         +  noisebuf[i + 16] / 16.0f;
}

int
math::nearest_pow2(int n)
{
    return pow(2, (int)(log(n) / log(2) + .5f));
}

float
math::deadzone(float value, float size)
{
    return (value < 0.0f)
        ? math::min(0.0f, value + size)
        : math::max(0.0f, value - size);
}
