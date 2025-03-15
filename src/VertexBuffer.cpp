#include "../include/VertexBuffer.hpp"
#include "../include/Renderer.hpp"

VertexBuffer::VertexBuffer(const void *data, unsigned int size) {
    GlCall(glGenBuffers(1, &_rendererId));
    GlCall(glBindBuffer(GL_ARRAY_BUFFER, _rendererId));
    GlCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}
VertexBuffer::~VertexBuffer() {
    GlCall(glDeleteBuffers(1, &_rendererId))
}

void VertexBuffer::Bind() const {
    GlCall(glBindBuffer(GL_ARRAY_BUFFER, _rendererId));
}
void VertexBuffer::Unbind() const {
    GlCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}