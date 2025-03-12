#include "../include/Face.hpp"
#include "../include/colors.hpp"
#include <iostream>

Face::Face() : _vertexIndices(), _normalIndices(), _textureIndices() {
    std::cout << YELLOW << "🛠️ Default Face Constructor called 🛠️" << RESET << std::endl;
}
Face::Face(const std::array<int, 3> &vertexIndices, const std::array<int, 3> &textureIndices,
           const std::array<int, 3> &normalIndices)
    : _vertexIndices(vertexIndices), _normalIndices(normalIndices), _textureIndices(textureIndices) {
    std::cout << YELLOW << "🛠️ Parameterized Face Constructor called 🛠️" << RESET << std::endl;
}
Face::~Face() {
    std::cout << RED << "🧨 Face Destructor called 🧨" << RESET << std::endl;
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