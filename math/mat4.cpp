#ifdef USE_OPENGL
#   define GLEW_STATIC
#   define GL3_PROTOTYPES 1
#   include <GL/glew.h>
#endif

#include "mat4.h"

#include <cmath>
#include <cstring> // memcpy

using namespace math;

Mat4::Mat4(const Mat4 &M)
{
    memcpy(this->data, M.data, sizeof(float) * 16);
}

Mat4::Mat4(const float *data)
{
    memcpy(this->data, data, sizeof(float) * 16);
}

Mat4
Mat4::identity(void)
{
    Mat4 M;
    M[ 0] = 1.0f; M[ 1] = 0.0f; M[ 2] = 0.0f; M[ 3] = 0.0f;
    M[ 4] = 0.0f; M[ 5] = 1.0f; M[ 6] = 0.0f; M[ 7] = 0.0f;
    M[ 8] = 0.0f; M[ 9] = 0.0f; M[10] = 1.0f; M[11] = 0.0f;
    M[12] = 0.0f; M[13] = 0.0f; M[14] = 0.0f; M[15] = 1.0f;

    // memset(&M.data, 0, sizeof(float) * 16);
    // M[ 0] = 1.0f;
    // M[ 5] = 1.0f;
    // M[10] = 1.0f;
    // M[15] = 1.0f;
    
    return M;
}

Mat4
Mat4::translation(const Vec3 &v)
{
    Mat4 M;
    M[ 0] = 1.0f; M[ 1] = 0.0f; M[ 2] = 0.0f; M[ 3] = 0.0f;
    M[ 4] = 0.0f; M[ 5] = 1.0f; M[ 6] = 0.0f; M[ 7] = 0.0f;
    M[ 8] = 0.0f; M[ 9] = 0.0f; M[10] = 1.0f; M[11] = 0.0f;
    M[12] = v.x;  M[13] = v.y;  M[14] = v.z;  M[15] = 1.0f;
    
    return M;
}

Mat4
Mat4::scaling(const Vec3 &v)
{
    Mat4 M;
    M[ 0] = v.x;  M[ 1] = 0.0f; M[ 2] = 0.0f; M[ 3] = 0.0f;
    M[ 4] = 0.0f; M[ 5] = v.y;  M[ 6] = 0.0f; M[ 7] = 0.0f;
    M[ 8] = 0.0f; M[ 9] = 0.0f; M[10] = v.z;  M[11] = 0.0f;
    M[12] = 0.0f; M[13] = 0.0f; M[14] = 0.0f; M[15] = 1.0f;
    
    return M;
}

Mat4
Mat4::rotationX(float angle)
{
    Mat4 M;
    float c = cos(angle), s = sin(angle);
    M[ 0] = 1.0f; M[ 1] = 0.0f; M[ 2] = 0.0f; M[ 3] = 0.0f;
    M[ 4] = 0.0f; M[ 5] = +c;   M[ 6] = -s;   M[ 7] = 0.0f;
    M[ 8] = 0.0f; M[ 9] = +s;   M[10] = +c;   M[11] = 0.0f;
    M[12] = 0.0f; M[13] = 0.0f; M[14] = 0.0f; M[15] = 1.0f;
    
    return M;
}

Mat4
Mat4::rotationY(float angle)
{
    Mat4 M;
    float c = cos(angle), s = sin(angle);
    M[ 0] = +c;   M[ 1] = 0.0f; M[ 2] = +s;   M[ 3] = 0.0f;
    M[ 4] = 0.0f; M[ 5] = 1.0f; M[ 6] = 0.0f; M[ 7] = 0.0f;
    M[ 8] = -s;   M[ 9] = 0.0f; M[10] = +c;   M[11] = 0.0f;
    M[12] = 0.0f; M[13] = 0.0f; M[14] = 0.0f; M[15] = 1.0f;
    
    return M;
}

Mat4
Mat4::rotationZ(float angle)
{
    Mat4 M;
    float c = cos(angle), s = sin(angle);
    M[ 0] = +c;   M[ 1] = -s;   M[ 2] = 0.0f; M[ 3] = 0.0f;
    M[ 4] = +s;   M[ 5] = +c;   M[ 6] = 0.0f; M[ 7] = 0.0f;
    M[ 8] = 0.0f; M[ 9] = 0.0f; M[10] = 1.0f; M[11] = 0.0f;
    M[12] = 0.0f; M[13] = 0.0f; M[14] = 0.0f; M[15] = 1.0f;
    
    return M;
}

Mat4
Mat4::perspective(float fovy, float aspect, float near, float far)
{
    Mat4 M;

    float
        f     = 1.0f / tan(fovy / 2.0f),
        depth = near - far;
    
    M[ 0] = f / aspect;
    M[ 1] = 0.0f;
    M[ 2] = 0.0f;
    M[ 3] = 0.0f;

    M[ 4] = 0.0f;
    M[ 5] = f;
    M[ 6] = 0.0f;
    M[ 7] = 0.0f;

    M[ 8] = 0.0f;
    M[ 9] = 0.0f;
    // M[10] = (-near - far) / depth;
    M[10] = (far + near) / depth;
    M[11] = -1.0f;

    M[12] = 0.0f;
    M[13] = 0.0f;
    M[14] = 2.0f * far * near / depth;
    M[15] = 0.0f;

    return M;
}

Mat4
Mat4::ortho(float left, float right, float top, float bottom, float near, float far)
{
    Mat4 M;
    float
        width  = right - left,
        height = top - bottom,
        depth  = far - near;

    M[ 0] = 2.0f / width;
    M[ 1] = 0.0f;
    M[ 2] = 0.0f;
    M[ 3] = 0.0f;

    M[ 4] = 0.0f;
    M[ 5] = 2.0f / height;
    M[ 6] = 0.0f;
    M[ 7] = 0.0f;

    M[ 8] = 0.0f;
    M[ 9] = 0.0f;
    M[10] = -2.0f / depth;
    M[11] = -1.0f;

    M[12] = -(right + left)   / width;
    M[13] = -(top   + bottom) / height;
    M[14] = -(far   + near)   / depth;
    M[15] = 1.0f;

    return M;
}

Mat4
Mat4::operator*(const Mat4 &M)
const
{
    Mat4 dst;
    const float *A = this->data;
    
    dst[ 0] = A[ 0] * M[ 0] + A[ 1] * M[ 4] + A[ 2] * M[ 8] + A[ 3] * M[12];
    dst[ 1] = A[ 0] * M[ 1] + A[ 1] * M[ 5] + A[ 2] * M[ 9] + A[ 3] * M[13];
    dst[ 2] = A[ 0] * M[ 2] + A[ 1] * M[ 6] + A[ 2] * M[10] + A[ 3] * M[14];
    dst[ 3] = A[ 0] * M[ 3] + A[ 1] * M[ 7] + A[ 2] * M[11] + A[ 3] * M[15];

    dst[ 4] = A[ 4] * M[ 0] + A[ 5] * M[ 4] + A[ 6] * M[ 8] + A[ 7] * M[12];
    dst[ 5] = A[ 4] * M[ 1] + A[ 5] * M[ 5] + A[ 6] * M[ 9] + A[ 7] * M[13];
    dst[ 6] = A[ 4] * M[ 2] + A[ 5] * M[ 6] + A[ 6] * M[10] + A[ 7] * M[14];
    dst[ 7] = A[ 4] * M[ 3] + A[ 5] * M[ 7] + A[ 6] * M[11] + A[ 7] * M[15];

    dst[ 8] = A[ 8] * M[ 0] + A[ 9] * M[ 4] + A[10] * M[ 8] + A[11] * M[12];
    dst[ 9] = A[ 8] * M[ 1] + A[ 9] * M[ 5] + A[10] * M[ 9] + A[11] * M[13];
    dst[10] = A[ 8] * M[ 2] + A[ 9] * M[ 6] + A[10] * M[10] + A[11] * M[14];
    dst[11] = A[ 8] * M[ 3] + A[ 9] * M[ 7] + A[10] * M[11] + A[11] * M[15];

    dst[12] = A[12] * M[ 0] + A[13] * M[ 4] + A[14] * M[ 8] + A[15] * M[12];
    dst[13] = A[12] * M[ 1] + A[13] * M[ 5] + A[14] * M[ 9] + A[15] * M[13];
    dst[14] = A[12] * M[ 2] + A[13] * M[ 6] + A[14] * M[10] + A[15] * M[14];
    dst[15] = A[12] * M[ 3] + A[13] * M[ 7] + A[14] * M[11] + A[15] * M[15];
    
    return dst;
}

Mat4 &
Mat4::operator*=(const Mat4 &M)
{
    float dst[15];
    float *A = this->data;
    
    dst[ 0] = A[ 0] * M[ 0] + A[ 1] * M[ 4] + A[ 2] * M[ 8] + A[ 3] * M[12];
    dst[ 1] = A[ 0] * M[ 1] + A[ 1] * M[ 5] + A[ 2] * M[ 9] + A[ 3] * M[13];
    dst[ 2] = A[ 0] * M[ 2] + A[ 1] * M[ 6] + A[ 2] * M[10] + A[ 3] * M[14];
    dst[ 3] = A[ 0] * M[ 3] + A[ 1] * M[ 7] + A[ 2] * M[11] + A[ 3] * M[15];

    dst[ 4] = A[ 4] * M[ 0] + A[ 5] * M[ 4] + A[ 6] * M[ 8] + A[ 7] * M[12];
    dst[ 5] = A[ 4] * M[ 1] + A[ 5] * M[ 5] + A[ 6] * M[ 9] + A[ 7] * M[13];
    dst[ 6] = A[ 4] * M[ 2] + A[ 5] * M[ 6] + A[ 6] * M[10] + A[ 7] * M[14];
    dst[ 7] = A[ 4] * M[ 3] + A[ 5] * M[ 7] + A[ 6] * M[11] + A[ 7] * M[15];

    dst[ 8] = A[ 8] * M[ 0] + A[ 9] * M[ 4] + A[10] * M[ 8] + A[11] * M[12];
    dst[ 9] = A[ 8] * M[ 1] + A[ 9] * M[ 5] + A[10] * M[ 9] + A[11] * M[13];
    dst[10] = A[ 8] * M[ 2] + A[ 9] * M[ 6] + A[10] * M[10] + A[11] * M[14];
    dst[11] = A[ 8] * M[ 3] + A[ 9] * M[ 7] + A[10] * M[11] + A[11] * M[15];

    dst[12] = A[12] * M[ 0] + A[13] * M[ 4] + A[14] * M[ 8] + A[15] * M[12];
    dst[13] = A[12] * M[ 1] + A[13] * M[ 5] + A[14] * M[ 9] + A[15] * M[13];
    dst[14] = A[12] * M[ 2] + A[13] * M[ 6] + A[14] * M[10] + A[15] * M[14];

    this->data[15]
            = A[12] * M[ 3] + A[13] * M[ 7] + A[14] * M[11] + A[15] * M[15];
    
    memcpy(A, dst, sizeof(float) * 15);
    
    return *this;
}

Mat4
Mat4::operator*(const Quat &q)
const
{
    return *this * q.mat4();
}

Mat4 &
Mat4::operator*=(const Quat &q)
{
    return *this *= q.mat4();
}


Mat4
Mat4::inverse(void)
const
{
    Mat4 I;
    const float *M = this->data;
    
    I[0] =
          M[ 5] * M[10] * M[15] - M[ 5] * M[11] * M[14]
        - M[ 9] * M[ 6] * M[15] + M[ 9] * M[ 7] * M[14]
        + M[13] * M[ 6] * M[11] - M[13] * M[ 7] * M[10];
        
    I[1] =
        - M[ 1] * M[10] * M[15] + M[ 1] * M[11] * M[14]
        + M[ 9] * M[ 2] * M[15] - M[ 9] * M[ 3] * M[14]
        - M[13] * M[ 2] * M[11] + M[13] * M[ 3] * M[10];
        
    I[2] =
          M[ 1] * M[ 6] * M[15] - M[ 1] * M[ 7] * M[14]
        - M[ 5] * M[ 2] * M[15] + M[ 5] * M[ 3] * M[14]
        + M[13] * M[ 2] * M[ 7] - M[13] * M[ 3] * M[ 6];
        
    I[3] =
        - M[ 1] * M[ 6] * M[11] + M[ 1] * M[ 7] * M[10]
        + M[ 5] * M[ 2] * M[11] - M[ 5] * M[ 3] * M[10]
        - M[ 9] * M[ 2] * M[ 7] + M[ 9] * M[ 3] * M[ 6];
        
    I[4] =
        - M[ 4] * M[10] * M[15] + M[ 4] * M[11] * M[14]
        + M[ 8] * M[ 6] * M[15] - M[ 8] * M[ 7] * M[14]
        - M[12] * M[ 6] * M[11] + M[12] * M[ 7] * M[10];
        
    I[5] =
          M[ 0] * M[10] * M[15] - M[ 0] * M[11] * M[14]
        - M[ 8] * M[ 2] * M[15] + M[ 8] * M[ 3] * M[14]
        + M[12] * M[ 2] * M[11] - M[12] * M[ 3] * M[10];
        
    I[6] =
        - M[ 0] * M[ 6] * M[15] + M[ 0] * M[ 7] * M[14]
        + M[ 4] * M[ 2] * M[15] - M[ 4] * M[ 3] * M[14]
        - M[12] * M[ 2] * M[ 7] + M[12] * M[ 3] * M[ 6];
        
    I[7] =
          M[ 0] * M[ 6] * M[11] - M[ 0] * M[ 7] * M[10]
        - M[ 4] * M[ 2] * M[11] + M[ 4] * M[ 3] * M[10]
        + M[ 8] * M[ 2] * M[ 7] - M[ 8] * M[ 3] * M[ 6];
        
    I[8] =
          M[ 4] * M[ 9] * M[15] - M[ 4] * M[11] * M[13]
        - M[ 8] * M[ 5] * M[15] + M[ 8] * M[ 7] * M[13]
        + M[12] * M[ 5] * M[11] - M[12] * M[ 7] * M[ 9];
        
    I[9] =
        - M[ 0] * M[ 9] * M[15] + M[ 0] * M[11] * M[13]
        + M[ 8] * M[ 1] * M[15] - M[ 8] * M[ 3] * M[13]
        - M[12] * M[ 1] * M[11] + M[12] * M[ 3] * M[ 9];
        
    I[10] =
          M[ 0] * M[ 5] * M[15] - M[ 0] * M[ 7] * M[13]
        - M[ 4] * M[ 1] * M[15] + M[ 4] * M[ 3] * M[13]
        + M[12] * M[ 1] * M[ 7] - M[12] * M[ 3] * M[ 5];
        
    I[11] =
        - M[ 0] * M[ 5] * M[11] + M[ 0] * M[ 7] * M[ 9]
        + M[ 4] * M[ 1] * M[11] - M[ 4] * M[ 3] * M[ 9]
        - M[ 8] * M[ 1] * M[ 7] + M[ 8] * M[ 3] * M[ 5];
        
    I[12] =
        - M[ 4] * M[ 9] * M[14] + M[ 4] * M[10] * M[13]
        + M[ 8] * M[ 5] * M[14] - M[ 8] * M[ 6] * M[13]
        - M[12] * M[ 5] * M[10] + M[12] * M[ 6] * M[ 9];
        
    I[13] =
          M[ 0] * M[ 9] * M[14] - M[ 0] * M[10] * M[13]
        - M[ 8] * M[ 1] * M[14] + M[ 8] * M[ 2] * M[13]
        + M[12] * M[ 1] * M[10] - M[12] * M[ 2] * M[ 9];
        
    I[14] =
        - M[ 0] * M[ 5] * M[14] + M[ 0] * M[ 6] * M[13]
        + M[ 4] * M[ 1] * M[14] - M[ 4] * M[ 2] * M[13]
        - M[12] * M[ 1] * M[ 6] + M[12] * M[ 2] * M[ 5];
        
    I[15] =
          M[ 0] * M[ 5] * M[10] - M[ 0] * M[ 6] * M[ 9]
        - M[ 4] * M[ 1] * M[10] + M[ 4] * M[ 2] * M[ 9]
        + M[ 8] * M[ 1] * M[ 6] - M[ 8] * M[ 2] * M[ 5];

    float determinant = 1.0f / (
          M[0] * I[0]
        + M[1] * I[4]
        + M[2] * I[8]
        + M[3] * I[12]
    );

    for (int i = 0; i < 16; ++i)
    {
        I[i] *= determinant;
    }

    return I;
}

Mat4
Mat4::transpose(void)
const
{
    Mat4 T;
    const float *M = this->data;
    
    T[ 0] = M[ 0]; T[ 1] = M[ 4]; T[ 2] = M[ 8]; T[ 3] = M[12];
    T[ 4] = M[ 1]; T[ 5] = M[ 5]; T[ 6] = M[ 9]; T[ 7] = M[13];
    T[ 8] = M[ 2]; T[ 9] = M[ 6]; T[10] = M[10]; T[11] = M[14];
    T[12] = M[ 3]; T[13] = M[ 7]; T[14] = M[11]; T[15] = M[15];
    
    return T;
}

Mat4 &
Mat4::reset_translation(void)
{
    float *M = this->data;
    M[12] = 0.0; M[13] = 0.0; M[14] = 0.0;
    
    return *this;
}

Mat4 &
Mat4::reset_rotation(void)
{
    float *M = this->data;
    M[ 0] = 1.0f; M[ 1] = 0.0f; M[ 2] = 0.0f; M[ 3] = 0.0f;
    M[ 4] = 0.0f; M[ 5] = 1.0f; M[ 6] = 0.0f; M[ 7] = 0.0f;
    M[ 8] = 0.0f; M[ 9] = 0.0f; M[10] = 1.0f; M[11] = 0.0f;
                                              M[15] = 1.0f;
    
    return *this;
}

float
Mat4::rotZ(void)
{
    return atan2(-this->data[8], this->data[0]);
}

float
Mat4::rotX(void)
{
    return asin(this->data[4]);
}

float
Mat4::rotY(void)
{
    return atan2(-this->data[6], this->data[5]);
}

Mat4 &
Mat4::translate(const Vec3 &v)
{
    //  1,  0,  0,  0,
    //  0,  1,  0,  0,
    //  0,  0,  1,  0,
    //  x,  y,  z,  1

    this->data[ 0] += this->data[ 3] * v.x;
    this->data[ 1] += this->data[ 3] * v.y;
    this->data[ 2] += this->data[ 3] * v.z;

    this->data[ 4] += this->data[ 7] * v.x;
    this->data[ 5] += this->data[ 7] * v.y;
    this->data[ 6] += this->data[ 7] * v.z;

    this->data[ 8] += this->data[11] * v.x;
    this->data[ 9] += this->data[11] * v.y;
    this->data[10] += this->data[11] * v.z;

    this->data[12] += this->data[15] * v.x;
    this->data[13] += this->data[15] * v.y;
    this->data[14] += this->data[15] * v.z;
    
    return *this;
}

Mat4 &
Mat4::scale(const Vec3 &v)
{
    //  x,  0,  0,  0,
    //  0,  y,  0,  0,
    //  0,  0,  z,  0,
    //  0,  0,  0,  1

    this->data[ 0] *= v.x;
    this->data[ 1] *= v.y;
    this->data[ 2] *= v.z;

    this->data[ 4] *= v.x;
    this->data[ 5] *= v.y;
    this->data[ 6] *= v.z;

    this->data[ 8] *= v.x;
    this->data[ 9] *= v.y;
    this->data[10] *= v.z;

    this->data[12] *= v.x;
    this->data[13] *= v.y;
    this->data[14] *= v.z;

    return *this;
}

Mat4 &
Mat4::rotX(float angle)
{
    //  1,  0,  0,  0,
    //  0,  c, -s,  0,
    //  0,  s,  c,  0,
    //  0,  0,  0,  1

    float
        f,
        c = cos(angle),
        s = sin(angle);

    f = this->data[1];
    this->data[ 1] = this->data[ 2] * s + f * c;
    this->data[ 2] = this->data[ 2] * c - f * s;

    f = this->data[5];
    this->data[ 5] = this->data[ 6] * s + f * c;
    this->data[ 6] = this->data[ 6] * c - f * s;

    f = this->data[9];
    this->data[ 9] = this->data[10] * s + f * c;
    this->data[10] = this->data[10] * c - f * s;

    f = this->data[13];
    this->data[13] = this->data[14] * s + f * c;
    this->data[14] = this->data[14] * c - f * s;

    return *this;
}

Mat4 &
Mat4::rotY(float angle)
{
    //  c,  0,  s,  0,
    //  0,  1,  0,  0,
    // -s,  0,  c,  0,
    //  0,  0,  0,  1

    float
        f,
        c = cos(angle),
        s = sin(angle);

    f = this->data[0];
    this->data[ 0] = f * c - this->data[ 2] * s;
    this->data[ 2] = f * s + this->data[ 2] * c;

    f = this->data[4];
    this->data[ 4] = f * c - this->data[ 6] * s;
    this->data[ 6] = f * s + this->data[ 6] * c;

    f = this->data[8];
    this->data[ 8] = f * c - this->data[10] * s;
    this->data[10] = f * s + this->data[10] * c;

    f = this->data[12];
    this->data[12] = f * c - this->data[14] * s;
    this->data[14] = f * s + this->data[14] * c;

    return *this;
}

Mat4 &
Mat4::rotZ(float angle)
{
    //  c, -s,  0,  0,
    //  s,  c,  0,  0,
    //  0,  0,  1,  0,
    //  0,  0,  0,  1

    float
        f,
        c = cos(angle),
        s = sin(angle);

    f = this->data[0];
    this->data[ 0] = this->data[ 1] * s + f * c;
    this->data[ 1] = this->data[ 1] * c - f * s;

    f = this->data[4];
    this->data[ 4] = this->data[ 5] * s + f * c;
    this->data[ 5] = this->data[ 5] * c - f * s;

    f = this->data[8];
    this->data[ 8] = this->data[ 9] * s + f * c;
    this->data[ 9] = this->data[ 9] * c - f * s;

    f = this->data[12];
    this->data[12] = this->data[13] * s + f * c;
    this->data[13] = this->data[13] * c - f * s;

    return *this;
}

#ifdef USE_OPENGL
Mat4 &
Mat4::to(GLint uniform)
{
    GLfloat f[16];
    for (int i = 0; i < 16; ++i)
    {
        f[i] = this->data[i];
    }
    glUniformMatrix4fv(uniform, 1, GL_FALSE, f);
    
    return *this;
}
#endif
