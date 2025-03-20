#include "../include/Model.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

Model::Model()
    : _vertex(), _textureCoordinates(), _vertexIndices(), _textureCoordinatesIndices(), _combinedVertexBuffer(),
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

    // Determine the bounding box.
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

    // Ensure that there is the same texture coordinate indices as vertex.
    _textureCoordinatesIndices.resize(_vertex.size());
    for (size_t i = 0; i < _vertex.size(); ++i) {
        _textureCoordinatesIndices[i] = i;
    }
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

        if (type == "v" || type == "vt") {
            std::istringstream iss(line.substr(start + 1));
            std::vector<float> values;
            float value;

            while (iss >> value) {
                values.push_back(value);
            }

            if (type == "v") {
                const Vector3 vec3(values[0], values[1], values[2]);
                _vertex.push_back(vec3);

            } else if (type == "vt") {
                const Vector2 vec2(values[0], values[1]);
                _textureCoordinates.push_back(vec2);
            }
        }

        else if (type == "f") {

            // At this state, we have parsed all vertices and texture coordinates. So if the container is empty, it
            // means we should calculate them.
            if (_textureCoordinates.empty()) {
                calculateTextureCoordinates();
            }

            std::istringstream iss(line.substr(start + 1));
            std::vector<std::string> vertices;
            std::string vertex;

            // Split each vertex in a string.
            // Ex: f 1/1/1 5/2/1 7/3/1 3/4/1 -> vertices[0] = 1/1/1.
            while (iss >> vertex) {
                vertices.push_back(vertex);
            }

            // If there is only 3 vertices, we can draw a triangle.
            if (vertices.size() == 3) {

                // Map to store unique vertex-texture combinations.
                // Ensure that a vertex unique. It's more efficient since we have less vertex to store and for me it
                // allows me to display a cube correctly x).
                std::unordered_map<std::string, unsigned int> uniqueVertices;

                for (const auto &triangleVertex : vertices) {
                    std::istringstream vertexStream(triangleVertex);
                    std::string index;
                    unsigned int vertexIndex = 0, textureIndex = 0;

                    // Parse vertices indices. (first /)
                    if (std::getline(vertexStream, index, '/')) {
                        if (!index.empty()) {
                            vertexIndex = std::stoul(index) - 1;
                        }
                    }

                    // Parse texture coordinates indices. (second /)
                    if (std::getline(vertexStream, index, '/')) {
                        if (!index.empty()) {
                            textureIndex = std::stoul(index) - 1;
                        }
                    }

                    // Generate a unique key for the vertex-texture combination.
                    std::string key = std::to_string(vertexIndex) + "/" + std::to_string(textureIndex);

                    // Check if the vertex-texture combination is not already in the map.
                    if (uniqueVertices.find(key) == uniqueVertices.end()) {

                        // Retrieve the vertices corresponding to vertices index found in face.
                        const Vector3 &vertex = _vertex[vertexIndex];
                        _combinedVertexBuffer.push_back(vertex.x);
                        _combinedVertexBuffer.push_back(vertex.y);
                        _combinedVertexBuffer.push_back(vertex.z);

                        // Retrieve the texture coordinates corresponding to vertices index found in face.
                        const Vector2 &texCoord = _textureCoordinates[textureIndex];
                        _combinedVertexBuffer.push_back(texCoord.u);
                        _combinedVertexBuffer.push_back(texCoord.v);

                        // Store the index of the new vertex by calculating last vertex index.
                        // Since a contain  5 float, we divide per 5 to know how many vertex are in there.
                        // - 1 for finding the new index.
                        uniqueVertices[key] = _combinedVertexBuffer.size() / 5 - 1;
                    }

                    // Store in indices container, It will be usefull to create Index buffer,
                    _vertexIndices.push_back(uniqueVertices[key]);
                }
            }

            // Apply fan triangulation for polygons with more than 3 vertices.
            else if (vertices.size() > 3) {

                // Map to store unique vertex-texture combinations.
                // Ensure that a vertex unique. It's more efficient since we have less vertex to store and for me it
                // allows me to display a cube correctly x).
                std::unordered_map<std::string, unsigned int> uniqueVertices;

                for (unsigned int i = 1; i < vertices.size() - 1; ++i) {

                    std::string triangleVertices[] = {vertices[0], vertices[i], vertices[i + 1]};

                    for (const auto &triangleVertex : triangleVertices) {
                        std::istringstream vertexStream(triangleVertex);
                        std::string index;
                        unsigned int vertexIndex = 0, textureIndex = 0;

                        // Parse vertices indices. (first /)
                        if (std::getline(vertexStream, index, '/')) {
                            if (!index.empty()) {
                                vertexIndex = std::stoul(index) - 1;
                            }
                        }

                        // Parse texture coordinates indices. (second /)
                        if (std::getline(vertexStream, index, '/')) {
                            if (!index.empty()) {
                                textureIndex = std::stoul(index) - 1;
                            }
                        }

                        // Generate a unique key for the vertex-texture combination.
                        std::string key = std::to_string(vertexIndex) + "/" + std::to_string(textureIndex);

                        // Check if the vertex-texture combination is not already in the map.
                        if (uniqueVertices.find(key) == uniqueVertices.end()) {

                            // Retrieve the vertices corresponding to vertices index found in face.
                            const Vector3 &vertex = _vertex[vertexIndex];
                            _combinedVertexBuffer.push_back(vertex.x);
                            _combinedVertexBuffer.push_back(vertex.y);
                            _combinedVertexBuffer.push_back(vertex.z);

                            // Retrieve the texture coordinates corresponding to vertices index found in face.
                            const Vector2 &texCoord = _textureCoordinates[textureIndex];
                            _combinedVertexBuffer.push_back(texCoord.u);
                            _combinedVertexBuffer.push_back(texCoord.v);

                            // Store the index of the new vertex by calculating last vertex index.
                            // Since a contain  5 float, we divide per 5 to know how many vertex are in there.
                            // - 1 for finding the new index.
                            uniqueVertices[key] = _combinedVertexBuffer.size() / 5 - 1;
                        }

                        // Store in indices container, It will be usefull to create Index buffer,
                        _vertexIndices.push_back(uniqueVertices[key]);
                    }
                }
            }
        }
    }
}