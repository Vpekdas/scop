#include "../include/Vector4.hpp"

Vector4::Vector4() : _x(0), _y(0), _z(0), _w(0) {
}
Vector4::~Vector4() {
}
Vector4::Vector4(float x, float y, float z, float w) {
    _x = x;
    _y = y;
    _z = z;
    _w = w;
}

float Vector4::getX() const {
    return _x;
}

float Vector4::getY() const {
    return _y;
}

float Vector4::getZ() const {
    return _z;
}

float Vector4::getW() const {
    return _w;
}