#include "../include/Model.hpp"
#include "../include/colors.hpp"
#include <fstream>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>

Model::Model()
    : _name(), _vertex(), _vertexNormals(), _textureCoordinates(), _faces(), _smoothingGroups(), _materials() {
    std::cout << YELLOW << "🛠️ Default Model Constructor called 🛠️" << RESET << std::endl;
}

Model::~Model() {
    std::cout << RED << "🧨 Model Destructor called 🧨" << RESET << std::endl;
}

const std::string &Model::getName() const {
    return _name;
}

const std::vector<Vector3> &Model::getVertex() const {
    return _vertex;
}

const std::vector<Vector3> &Model::getVertexNormals() const {
    return _vertexNormals;
}

const std::vector<Vector2> &Model::getTextureCoordinates() const {
    return _textureCoordinates;
}

const std::vector<Face> &Model::getFaces() const {
    return _faces;
}

const std::vector<int> &Model::getSmoothingGroups() const {
    return _smoothingGroups;
}
const std::vector<std::string> &Model::getMaterials() const {
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
    std::size_t spaceIndex = 0, nextSpaceIndex = 0, lastSpaceIndex = 0;
    float x = 0.0, y = 0.0, z = 0.0;

    while (std::getline(infile, line)) {
        spaceIndex = line.find(" ");
        type = line.substr(0, spaceIndex);

        if (type == "o") {
            _name = line.substr(spaceIndex + 1);
        }

        else if (type == "v") {

            spaceIndex = line.find(" ", type.length());
            nextSpaceIndex = line.find(" ", spaceIndex + 1);
            lastSpaceIndex = line.rfind(" ");

            x = std::stof(line.substr(spaceIndex, nextSpaceIndex));

            spaceIndex = line.find(" ", nextSpaceIndex);
            nextSpaceIndex = line.find(" ", spaceIndex + 1);
            y = std::stof(line.substr(spaceIndex, nextSpaceIndex));

            z = std::stof(line.substr(lastSpaceIndex));

            Vector3 vec3(x, y, z);
            _vertex.push_back(vec3);

        }

        else if (type == "vn") {

            spaceIndex = line.find(" ", type.length());
            nextSpaceIndex = line.find(" ", spaceIndex + 1);
            lastSpaceIndex = line.rfind(" ");

            x = std::stof(line.substr(spaceIndex, nextSpaceIndex));

            spaceIndex = line.find(" ", nextSpaceIndex);
            nextSpaceIndex = line.find(" ", spaceIndex + 1);
            y = std::stof(line.substr(spaceIndex, nextSpaceIndex));

            z = std::stof(line.substr(lastSpaceIndex));

            Vector3 vec3(x, y, z);
            _vertexNormals.push_back(vec3);
        }

        else if (type == "vt") {

            spaceIndex = line.find(" ", type.length());
            nextSpaceIndex = line.find(" ", spaceIndex + 1);
            lastSpaceIndex = line.rfind(" ");

            x = std::stof(line.substr(spaceIndex, nextSpaceIndex));

            y = std::stof(line.substr(lastSpaceIndex));

            Vector2 vec2(x, y);
            _textureCoordinates.push_back(vec2);
        }

        else if (type == "f") {

            // TODO
        }
    }
}