#pragma once

class Vector4 {
    public:
    Vector4();
    ~Vector4();
    Vector4(float x, float y, float z, float w);

    float getX() const;
    float getY() const;
    float getZ() const;
    float getW() const;

    private:
    float _x, _y, _z, _w;

    protected:
};