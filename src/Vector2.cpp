#include "../include/Vector2.hpp"

Vector2::Vector2() : _x(0), _y(0) {
}
Vector2::~Vector2() {
}
Vector2::Vector2(float x, float y) {
    _x = x;
    _y = y;
}

float Vector2::getX() const {
    return _x;
}

float Vector2::getY() const {
    return _y;
}