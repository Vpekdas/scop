#pragma once

// VertexBuffer is a buffer that contains unique vertex positions followed by
// their texture coordinates in my case. Since OpenGL is a state machine, we
// only need to bind the buffer, and it will handle the rendering.
class VertexBuffer {
    public:
    VertexBuffer(const void *data, unsigned int size);
    ~VertexBuffer();

    // Generate an GL_ARRAY_BUFFER, bind it and setting it to GL_STATIC_DRAW.
    void Bind() const;
    void Unbind() const;

    private:
    unsigned int _rendererId;
};
