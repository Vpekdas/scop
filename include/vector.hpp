#pragma once
#include <cmath>

struct Vector2 {
    Vector2() : u(0), v(0) {
    }
    Vector2(float u, float v) : u(u), v(v) {
    }

    float u, v;
};

struct Vector3 {
    Vector3() : x(0), y(0), z(0) {
    }
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {
    }

    Vector3 operator-() const {
        return Vector3(-x, -y, -z);
    }

    float x, y, z;
};