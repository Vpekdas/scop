#pragma once

#include "vector.hpp"
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


    // https://github.com/g-truc/glm/blob/master/glm/ext/matrix_clip_space.inl
    // perspectiveFovRH_NO
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
        Matrix4 m(1.0);
        m._m[3][0] = v.x;
        m._m[3][1] = v.y;
        m._m[3][2] = v.z;
        return m;
    }


    // https://www.geeksforgeeks.org/rotation-matrix/#3d-rotation-matrix

    // TODO: Overload operator for multypling vector.
    static Matrix4 rotationX(float angle) {
        Matrix4 m(1.0);
        float rad = angle * M_PI / 180.0;
        m._m[1][1] = cos(rad);
        m._m[1][2] = -sin(rad);
        m._m[2][1] = sin(rad);
        m._m[2][2] = cos(rad);
        return m;
    }

    static Matrix4 rotationY(float angle) {
        Matrix4 m(1.0);
        float rad = angle * M_PI / 180.0;
        m._m[0][0] = cos(rad);
        m._m[0][2] = sin(rad);
        m._m[2][0] = -sin(rad);
        m._m[2][2] = cos(rad);
        return m;
    }

    static Matrix4 rotationZ(float angle) {
        Matrix4 m(1.0);
        float rad = angle * M_PI / 180.0;
        m._m[0][0] = cos(rad);
        m._m[0][1] = -sin(rad);
        m._m[1][0] = sin(rad);
        m._m[1][1] = cos(rad);
        return m;
    }

    float _m[4][4] = {};
};
