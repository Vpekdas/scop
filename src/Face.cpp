#include "../include/Face.hpp"
#include <iostream>

Face::Face() : _vertexIndices(), _normalIndices(), _textureIndices() {
}
Face::Face(const std::vector<int> &vertexIndices, const std::vector<int> &textureIndices,
           const std::vector<int> &normalIndices)
    : _vertexIndices(vertexIndices), _normalIndices(normalIndices), _textureIndices(textureIndices) {
}
Face::~Face() {
}

const std::vector<int> &Face::getVertexIndices() const {
    return _vertexIndices;
}
const std::vector<int> &Face::getTextureIndices() const {
    return _textureIndices;
}
const std::vector<int> &Face::getNormalIndices() const {
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