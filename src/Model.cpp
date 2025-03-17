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
    : _name(), _vertex(), _vertexNormals(), _textureCoordinates(), _smoothingGroups(), _materials(),
      _centroid(0, 0, 0) {
}

void Model::calculateCentroid() {
    Vector3 sum(0, 0, 0);
    for (const auto &vertex : _vertex) {
        sum.x += vertex.x;
        sum.y += vertex.y;
        sum.z += vertex.z;
    }
    _centroid = Vector3(sum.x / _vertex.size(), sum.y / _vertex.size(), sum.z / _vertex.size());
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
            std::istringstream iss(line.substr(start + 1));
            std::vector<std::string> vertices;
            std::string vertex;

            while (iss >> vertex) {
                vertices.push_back(vertex);
            }

            // Triangulate if there is more than 3 faces.
            if (vertices.size() > 3) {
                for (size_t i = 1; i < vertices.size() - 1; i++) {
                    std::vector<unsigned int> indices;
                    for (size_t i = 1; i < vertices.size() - 1; i++) {
                        std::vector<unsigned int> indices;

                        size_t faceIndices[] = {0, i, i + 1};
                        for (size_t j = 0; j < 3; ++j) {
                            std::istringstream vertexStream(vertices[faceIndices[j]]);
                            std::string index;
                            unsigned int vertexIndex, textureIndex, normalIndex;

                            std::getline(vertexStream, index, '/');
                            vertexIndex = std::stoul(index) - 1;
                            indices.push_back(vertexIndex);
                            _vertexIndices.push_back(vertexIndex);

                            if (std::getline(vertexStream, index, '/')) {
                                if (!index.empty()) {
                                    textureIndex = std::stoul(index) - 1;
                                    _textureCoordinatesIndices.push_back(textureIndex);
                                } else {
                                    // TODO: Calculate if no texture coordinate.
                                }
                            }

                            if (std::getline(vertexStream, index, '/')) {
                                if (!index.empty()) {
                                    normalIndex = std::stoul(index) - 1;
                                    _vertexNormalsIndices.push_back(normalIndex);
                                } else {
                                    // TODO: Calculate if no vertices normals.
                                }
                            }
                        }
                    }
                }
            } else {
                for (const auto &vertex : vertices) {
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
                            // TODO: Calculate if no texture coordinate.
                        }
                    }

                    if (std::getline(vertexStream, index, '/')) {
                        if (!index.empty()) {
                            normalIndex = std::stoul(index) - 1;
                            _vertexNormalsIndices.push_back(normalIndex);
                        } else {
                            // TODO: Calculate if no vertices normals.
                        }
                    }
                }
            }
        }
    }
}