#pragma once

// An Index Buffer is an efficient way to render 3D models.
// Instead of storing duplicate vertices (positions and texture coordinates in
// my case), we store unique vertices and create indices to reference them. For
// example, a square consists of 2 triangles, where 2 vertices are shared and
// have the same attributes (positions and texture coordinates). By storing
// unique vertices, we only need to create 2 additional indices for the shared
// vertices. As a result, we store just 2 unsigned integers as indices and 5
// floats per vertex (3 for position and 2 for texture coordinates). Without an
// Index Buffer, we would store the same vertex multiple times in the buffer,
// which is less efficient in terms of memory usage.

class IndexBuffer {
    public:
    // Generate an GL_ELEMENT_ARRAY_BUFFER, bind it and setting it to
    // GL_STATIC_DRAW.
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
