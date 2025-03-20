#pragma once

struct Vector2 {
    Vector2(float u, float v);

    float u, v;
};

struct Vector3 {
    Vector3(float x, float y, float z);

    Vector3 operator-() const {
        return Vector3(-x, -y, -z);
    }

    float x, y, z;
};
