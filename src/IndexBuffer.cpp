#include "../include/IndexBuffer.hpp"
#include "../include/Renderer.hpp"

IndexBuffer::IndexBuffer(const unsigned int *data, unsigned int count) : _count(count) {
    GlCall(glGenBuffers(1, &_rendererId));
    GlCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _rendererId));
    GlCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW));
}
IndexBuffer::~IndexBuffer() {
    GlCall(glDeleteBuffers(1, &_rendererId))
}

void IndexBuffer::Bind() const {
    GlCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _rendererId));
}
void IndexBuffer::Unbind() const {
    GlCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}