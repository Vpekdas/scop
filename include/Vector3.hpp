#pragma once

class Vector3 {
    public:
    Vector3();
    ~Vector3();
    Vector3(float x, float y, float z);

    float getX() const;
    float getY() const;
    float getZ() const;

    private:
    float _x, _y, _z;

    protected:
};