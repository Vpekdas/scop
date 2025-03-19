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

void Model::calculateTextureCoordinates() {
    if (_vertex.empty())
        return;

    // Determine the bounding box
    Vector3 min = _vertex[0];
    Vector3 max = _vertex[0];
    for (const auto &vertex : _vertex) {
        if (vertex.x < min.x)
            min.x = vertex.x;
        if (vertex.y < min.y)
            min.y = vertex.y;
        if (vertex.z < min.z)
            min.z = vertex.z;
        if (vertex.x > max.x)
            max.x = vertex.x;
        if (vertex.y > max.y)
            max.y = vertex.y;
        if (vertex.z > max.z)
            max.z = vertex.z;
    }

    for (const auto &vertex : _vertex) {
        float u = (vertex.x - min.x) / (max.x - min.x);
        float v = (vertex.y - min.y) / (max.y - min.y);
        Vector2 texCoord(u, v);
        _textureCoordinates.push_back(texCoord);
    }

    _textureCoordinatesIndices.resize(_vertex.size());
    for (size_t i = 0; i < _vertex.size(); ++i) {
        _textureCoordinatesIndices[i] = i;
    }
}

void Model::createCombinedVertexBuffer() {

    for (size_t i = 0; i < _vertex.size(); i++) {

        const Vector3 &vertex = _vertex[i];
        const Vector2 &texCoord = _textureCoordinates[i];

        _combinedVertexBuffer.push_back(vertex.x);
        _combinedVertexBuffer.push_back(vertex.y);
        _combinedVertexBuffer.push_back(vertex.z);
        _combinedVertexBuffer.push_back(texCoord.x);
        _combinedVertexBuffer.push_back(texCoord.y);
    }

    const Vector3 &vertex = _vertex[5];
    const Vector3 &vertex2 = _vertex[4];
    const Vector3 &vertex3 = _vertex[0];

    const Vector2 &coordinate = _textureCoordinates[0];
    const Vector2 &coordinate2 = _textureCoordinates[1];
    const Vector2 &coordinate3 = _textureCoordinates[2];

    std::cout << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;
    std::cout << vertex2.x << " " << vertex2.y << " " << vertex2.z << std::endl;
    std::cout << vertex3.x << " " << vertex3.y << " " << vertex3.z << std::endl;

    std::cout << coordinate.x << " " << coordinate.y << " " << std::endl;
    std::cout << coordinate2.x << " " << coordinate2.y << " " << std::endl;
    std::cout << coordinate3.x << " " << coordinate3.y << " " << std::endl;
}

void Model::parse(const std::string &filename) {
    if (filename.find(".obj") == std::string::npos) {
        throw std::runtime_error("Error: file is not an obj.");
    }

    std::ifstream infile(filename);

    if (!infile.is_open()) {
        const std::string error = "Error: Cannot open file: " + filename + "\n";
        throw std::runtime_error(error);
    }

    std::string line, type;
    std::size_t start = 0;

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

                    unsigned long faceIndices[] = {0, i, i + 1};
                    for (size_t j = 0; j < 3; ++j) {
                        std::istringstream vertexStream(vertices[faceIndices[j]]);
                        std::string index;
                        unsigned int vertexIndex, textureIndex, normalIndex;

                        std::getline(vertexStream, index, '/');
                        vertexIndex = std::stoul(index) - 1;
                        _vertexIndices.push_back(vertexIndex);

                        if (std::getline(vertexStream, index, '/')) {
                            if (!index.empty()) {
                                textureIndex = std::stoul(index) - 1;
                                _textureCoordinatesIndices.push_back(textureIndex);
                            }
                        }

                        if (std::getline(vertexStream, index, '/')) {
                            if (!index.empty()) {
                                normalIndex = std::stoul(index) - 1;
                                _vertexNormalsIndices.push_back(normalIndex);
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
                        }
                    }

                    if (std::getline(vertexStream, index, '/')) {
                        if (!index.empty()) {
                            normalIndex = std::stoul(index) - 1;
                            _vertexNormalsIndices.push_back(normalIndex);
                        }
                    }
                }
            }
        }
    }

    if (_textureCoordinates.empty()) {
        calculateTextureCoordinates();
    }

    createCombinedVertexBuffer();
}