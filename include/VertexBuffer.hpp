#pragma once

#include <glad/glad.h>

#include "../include/vector.hpp"

class VertexBuffer {
    public:
    VertexBuffer(const void *data, unsigned int size);
    ~VertexBuffer();

    void Bind() const;
    void Unbind() const;

    private:
    unsigned int _rendererId;
};
