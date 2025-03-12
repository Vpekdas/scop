#include "../include/Model.hpp"
#include "../include/colors.hpp"
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

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
    std::size_t start = 0, end = 0;

    while (std::getline(infile, line)) {
        start = line.find(" ");
        type = line.substr(0, start);

        if (type == "o") {
            _name = line.substr(start + 1);
        }

        else if (type == "v" || type == "vn" || type == "vt") {
            std::istringstream iss(line.substr(start + 1));
            std::vector<float> values;
            float value;

            while (iss >> value) {
                values.push_back(value);
            }

            if (type == "v" || type == "vn") {
                Vector3 vec3(values[0], values[1], values[2]);
                if (type == "v") {
                    _vertex.push_back(vec3);
                } else {
                    _vertexNormals.push_back(vec3);
                }
            } else if (type == "vt") {
                Vector2 vec2(values[0], values[1]);
                _textureCoordinates.push_back(vec2);
            }
        }

        else if (type == "f") {
            std::vector<int> vertexIndices, textureIndices, normalIndices;
            std::istringstream iss(line.substr(start + 1));
            std::string vertex;

            while (iss >> vertex) {
                std::istringstream vertexStream(vertex);
                std::string index;
                int vertexIndex, textureIndex, normalIndex;

                std::getline(vertexStream, index, '/');
                vertexIndex = std::stoi(index) - 1;
                vertexIndices.push_back(vertexIndex);

                if (std::getline(vertexStream, index, '/')) {
                    if (!index.empty()) {
                        textureIndex = std::stoi(index) - 1;
                        textureIndices.push_back(textureIndex);
                    } else {
                        textureIndices.push_back(-1);
                    }
                }

                if (std::getline(vertexStream, index, '/')) {
                    if (!index.empty()) {
                        normalIndex = std::stoi(index) - 1;
                        normalIndices.push_back(normalIndex);
                    } else {
                        normalIndices.push_back(-1);
                    }
                }
            }

            Face face(vertexIndices, textureIndices, normalIndices);
            _faces.push_back(face);
        }
    }
}