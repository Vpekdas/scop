#pragma once

#include <glad/glad.h>

class IndexBuffer {
    public:
    IndexBuffer(const unsigned int *data, unsigned int count);
    ~IndexBuffer();

    void Bind() const;
    void Unbind() const;

    inline unsigned int getCount() const {
        return _count;
    }

    private:
    unsigned int _rendererId;
    unsigned int _count;
};
