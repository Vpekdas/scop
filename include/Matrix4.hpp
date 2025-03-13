#pragma once

#include "Vector3.hpp"
#include <cmath>
#include <iostream>

struct Matrix4 {
    Matrix4() : Matrix4(1.0) {
    }

    Matrix4(float v) {
        _m[0][0] = v;
        _m[1][1] = v;
        _m[2][2] = v;
        _m[3][3] = v;
    }

    static Matrix4 perspective(float fov, int width, int height, float near, float far) {
        const float rad = fov * M_PI / 180.0;
        const float h = cos(0.5 * fov) / sin(0.5 * fov);
        const float w = h * (float)height / (float)width;

        Matrix4 m(0.0);
        m._m[0][0] = w;
        m._m[1][1] = h;
        m._m[2][2] = -(far + near) / (far - near);
        m._m[2][3] = -1.0;
        m._m[3][2] = (-2.0 * far * near) / (far - near);

        return m;
    }

    static Matrix4 translation(const Vector3 &v) {
        Matrix4 m;
        m._m[3][0] = v.x;
        m._m[3][1] = v.y;
        m._m[3][2] = v.z;
        return m;
    }

    float _m[4][4] = {};
};
