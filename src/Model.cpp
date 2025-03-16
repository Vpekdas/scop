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
    : _name(), _vertex(), _vertexNormals(), _textureCoordinates(), _smoothingGroups(), _materials(), _minXVertex(0),
      _minYVertex(0), _maxXVertex(0), _maxYVertex(0), _center(0, 0, 0) {
}

void Model::findCenterAxis(float value, Axis axis) {
    if (axis == X) {
        if (value < _minXVertex) {
            _minXVertex = value;
        }
        if (value > _maxXVertex) {
            _maxXVertex = value;
        }
    } else if (axis == Y) {
        if (value < _minYVertex) {
            _minYVertex = value;
        }
        if (value > _maxYVertex) {
            _maxYVertex = value;
        }
    }
}

void Model::calculateCenterAxis() {
    float centerX = (_minXVertex + _maxXVertex) / 2.0f;
    float centerY = (_minYVertex + _maxYVertex) / 2.0f;
    _center = Vector3(centerX, centerY, 0.0f);
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
                    findCenterAxis(values[0], X);
                    findCenterAxis(values[1], Y);
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
            std::istringstream iss(line.substr(start + 1));
            std::string vertex;

            while (iss >> vertex) {
                std::istringstream vertexStream(vertex);
                std::string index;
                unsigned int vertexIndex, textureIndex, normalIndex;

                std::getline(vertexStream, index, '/');
                vertexIndex = std::stoul(index) - 1;
                _vertexIndices.push_back(vertexIndex);

                if (std::getline(vertexStream, index, '/')) {
                    if (!index.empty()) {
                        textureIndex = std::stoul(index) - 1;
                        _textureCoordinatesIndices.push_back(textureIndex);
                    } else {
                        // TODO: Handle if no texture coordinate.
                    }
                }

                if (std::getline(vertexStream, index, '/')) {
                    if (!index.empty()) {
                        normalIndex = std::stoul(index) - 1;
                        _vertexNormalsIndices.push_back(normalIndex);
                    } else {
                        // TODO: Handle if no vertices normals.
                    }
                }
            }
        }
    }
}