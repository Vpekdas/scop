#include "../include/Model.hpp"
#include "../include/colors.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>

Model::Model()
    : _name(), _vertex(), _vertexNormals(), _textureCoordinates(), _faces(), _smoothingGroups(), _materials() {
    std::cout << YELLOW << "🛠️ Default Model Constructor called 🛠️" << RESET << std::endl;
}

Model::~Model() {
    std::cout << RED << "🧨 Model Destructor called 🧨" << RESET << std::endl;
}

const std::string &Model::getName() {
    return _name;
}
const std::vector<float> &Model::getVertex() {
    return _vertex;
}
const std::vector<float> &Model::getVertexNormals() {
    return _vertexNormals;
}
const std::vector<float> &Model::getTextureCoordinates() {
    return _textureCoordinates;
}
const std::vector<int> &Model::getFaces() {
    return _faces;
}
const std::vector<int> &Model::getSmoothingGroups() {
    return _smoothingGroups;
}
const std::vector<std::string> &Model::getMaterials() {
    return _materials;
}

void Model::parse(const std::string &filename) {
    if (filename.find(".obj") == std::string::npos) {
        throw std::runtime_error(NEON_RED "Error: file is not an obj." RESET);
    }

    std::ifstream infile(filename);

    if (!infile.is_open()) {
        const std::string error = "Error: Cannot open file: " + filename + "\n";
        throw std::runtime_error(error);
    }

    std::string line, type;
    std::size_t spaceIndex = 0;

    while (std::getline(infile, line)) {
        spaceIndex = line.find(" ");
        type = line.substr(0, spaceIndex);
    }
}