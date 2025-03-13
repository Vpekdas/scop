#include "../include/Face.hpp"
#include <iostream>

Face::Face() : _vertexIndices(), _normalIndices(), _textureIndices() {
}
Face::Face(const std::array<int, 3> &vertexIndices, const std::array<int, 3> &textureIndices,
           const std::array<int, 3> &normalIndices)
    : _vertexIndices(vertexIndices), _normalIndices(normalIndices), _textureIndices(textureIndices) {
}
Face::~Face() {
}

const std::array<int, 3> &Face::getVertexIndices() const {
    return _vertexIndices;
}
const std::array<int, 3> &Face::getTextureIndices() const {
    return _textureIndices;
}
const std::array<int, 3> &Face::getNormalIndices() const {
    return _normalIndices;
}

std::ostream &operator<<(std::ostream &out, const Face &face) {
    out << "Vertex Indices: ";
    for (const auto &index : face.getVertexIndices()) {
        out << index << " ";
    }
    out << "\nTexture Indices: ";
    for (const auto &index : face.getTextureIndices()) {
        out << index << " ";
    }
    out << "\nNormal Indices: ";
    for (const auto &index : face.getNormalIndices()) {
        out << index << " ";
    }
    return out;
}