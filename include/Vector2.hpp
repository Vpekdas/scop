#pragma once

class Vector2 {
    public:
    Vector2();
    ~Vector2();
    Vector2(float x, float y);

    float getX() const;
    float getY() const;

    private:
    float _x, _y;

    protected:
};