#ifndef _MATH_MATRIX_4x4_H
#define _MATH_MATRIX_4x4_H

#include "vec3.h"
#include "quat.h"

#ifdef USE_OPENGL
#   include <GL/gl.h>
#endif

namespace math
{
    class Vec3;
    class Mat4;
    class Mat4
    {
        public:
            float data[16];

            // Identity matrix
            static Mat4 identity(void);

            // Translation matrices
            static Mat4 translation(const Vec3 &v);
            static Mat4 translation(float x, float y, float z = 0.0f) { return Mat4::translation(Vec3(x, y, z)); }
            static Mat4 scaling(const Vec3 &v);
            static Mat4 scaling(float x, float y, float z = 1.0f) { return Mat4::scaling(Vec3(x, y, z)); }
            static Mat4 scaling(float s) { return Mat4::scaling(Vec3(s, s, s)); }
            static Mat4 rotationX(float angle);
            static Mat4 rotationY(float angle);
            static Mat4 rotationZ(float angle);
            
            // Projection matrices
            static Mat4 perspective(float fovy, float aspect, float near, float far);
            static Mat4 ortho(float left, float right, float top, float bottom, float near, float far);

            // Constructors
            Mat4() {};
            Mat4(const Mat4 &M);
            Mat4(const float *data);
            Mat4(
                float aa, float ab, float ac, float ad,
                float ba, float bb, float bc, float bd,
                float ca, float cb, float cc, float cd,
                float da, float db, float dc, float dd
            ) { this->data[0] = aa; this->data[1] = ab; this->data[2] = ac; this->data[3] = ad; this->data[4] = ba; this->data[5] = bb; this->data[6] = bc; this->data[7] = bd; this->data[8] = ca; this->data[9] = cb; this->data[10] = cc; this->data[11] = cd; this->data[12] = da; this->data[13] = db; this->data[14] = dc; this->data[15] = dd; }
            
                  float &operator[](int i)       { return this->data[i]; }
            const float &operator[](int i) const { return this->data[i]; }

            Mat4 transpose() const;
            Mat4 inverse()   const;
            
            // Transformations
            Mat4 &translate(const Vec3 &v);
            Mat4 &translate(float x, float y, float z = 0.0f) { return this->translate(Vec3(x, y, z)); }
            Mat4 &translateX(float s) { return this->translate(Vec3(s, 0.0f, 0.0f)); }
            Mat4 &translateY(float s) { return this->translate(Vec3(0.0f, s, 0.0f)); }
            Mat4 &translateZ(float s) { return this->translate(Vec3(0.0f, 0.0f, s)); }

            Mat4 &scale(const Vec3 &v);
            Mat4 &scale(float x, float y, float z = 1.0f) { return this->scale(Vec3(x, y, z)); }
            Mat4 &scale(float s) { return this->scale(Vec3(s, s, s)); }
            Mat4 &scaleX(float s) { return this->scale(Vec3(s, 1.0f, 1.0f)); }
            Mat4 &scaleY(float s) { return this->scale(Vec3(1.0f, s, 1.0f)); }
            Mat4 &scaleZ(float s) { return this->scale(Vec3(1.0f, 1.0f, s)); }

            Mat4 &rotX(float angle);
            Mat4 &rotY(float angle);
            Mat4 &rotZ(float angle);

            // Get Euler angles for rotation
            float rotX(void);
            float rotY(void);
            float rotZ(void);
            
            float
            trace(void) const { return this->data[0] + this->data[5] + this->data[10]; }

            Mat4 &reset_translation(void);
            Mat4 &reset_rotation(void);

            // Multiplications
            Mat4 operator*(const Mat4 &M) const;
            Mat4 &operator*=(const Mat4 &M);
            Mat4 operator*(const Quat &q) const;
            Mat4 &operator*=(const Quat &q);

#   ifdef USE_OPENGL
            Mat4 &to(GLint uniform);
#   endif

    };
}

#endif
