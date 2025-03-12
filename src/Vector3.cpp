#include "../include/Vector3.hpp"

Vector3::Vector3() : _x(0), _y(0), _z(0) {
}
Vector3::~Vector3() {
}
Vector3::Vector3(float x, float y, float z) {
    _x = x;
    _y = y;
    _z = z;
}

float Vector3::getX() const {
    return _x;
}

float Vector3::getY() const {
    return _y;
}

float Vector3::getZ() const {
    return _z;
}