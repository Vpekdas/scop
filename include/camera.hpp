#pragma once

#include "../include/vector.hpp"

struct Camera {
    Vector3 pos;
    float rotationAngle = 0.0f;

    Camera(float x, float y, float z) {
        pos.x = x;
        pos.y = y;
        pos.z = z;
    }

    void Move(float deltaTime);
};