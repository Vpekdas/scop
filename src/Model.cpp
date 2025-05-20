#include "../include/Model.hpp"
#include <fstream>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

Model::Model()
    : _vertices(), _verticesIndices(), _textureCoordinates(), _textureIndices(), _vertexBuffer(), _uniqueVertices(),
      _triangleVertices(), _centroid(0, 0, 0) {

    _triangleVertices.reserve(3);
}

void Model::calculateCentroid() {
    if (_vertices.empty()) {
        _centroid = Vector3(0, 0, 0);
        return;
    }

    Vector3 sum =
        std::accumulate(_vertices.begin(), _vertices.end(), Vector3(0, 0, 0),
                        [](const Vector3 &a, const Vector3 &b) { return Vector3(a.x + b.x, a.y + b.y, a.z + b.z); });

    _centroid = Vector3(sum.x / _vertices.size(), sum.y / _vertices.size(), sum.z / _vertices.size());
}

// https://community.khronos.org/t/calc-texture-coordinate/14707/5

void Model::calculateTextureCoordinates() {
    if (_vertices.empty()) {
        throw std::runtime_error("Error: no vertices found in obj.");
    }

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();

    for (const Vector3 &vertex : _vertices) {
        minX = std::min(minX, vertex.x);
        maxX = std::max(maxX, vertex.x);
        minY = std::min(minY, vertex.y);
        maxY = std::max(maxY, vertex.y);
    }

    for (size_t i = 0; i < _vertices.size(); i += 3) {
        const Vector3 &vertex0 = _vertices[i];
        const Vector3 &vertex1 = _vertices[i + 1];
        const Vector3 &vertex2 = _vertices[i + 2];

        const Vector2 uv0((vertex0.x - minX) / (maxX - minX), (vertex0.y - minY) / (maxY - minY));
        const Vector2 uv1((vertex1.x - minX) / (maxX - minX), (vertex1.y - minY) / (maxY - minY));
        const Vector2 uv2((vertex2.x - minX) / (maxX - minX), (vertex2.y - minY) / (maxY - minY));

        _textureCoordinates.push_back(uv0);
        _textureCoordinates.push_back(uv1);
        _textureCoordinates.push_back(uv2);
    }

    for (size_t i = 0; i < _vertices.size(); i++) {
        _textureIndices.push_back(i);
    }
}

void Model::createFaces(const std::vector<std::string> &vertices) {
    for (const auto &triangleVertex : vertices) {
        std::istringstream vertexStream(triangleVertex);
        std::string index;
        unsigned int vertexIndex = 0, textureIndex = 0;

        // Parse vertices indices. (first /)
        if (std::getline(vertexStream, index, '/') && !index.empty()) {
            vertexIndex = std::stoul(index) - 1;
        }

        // Parse texture coordinates indices. (second /)
        if (std::getline(vertexStream, index, '/') && !index.empty()) {
            textureIndex = std::stoul(index) - 1;
        } else {
            // Protect the case that face is parsed but indices is not in file.
            if (vertexIndex >= _textureIndices.size()) {
                throw std::runtime_error("Error: Vertex index out of bounds");
            }
            textureIndex = _textureIndices[vertexIndex];
        }

        // Generate a unique key for the vertex-texture combination.
        const std::string key = std::to_string(vertexIndex) + "/" + std::to_string(textureIndex);

        if (_uniqueVertices.find(key) == _uniqueVertices.end()) {

            // Retrieve the vertices corresponding to vertices index found in face.
            const Vector3 &vertex = _vertices[vertexIndex];
            _vertexBuffer.push_back(vertex.x);
            _vertexBuffer.push_back(vertex.y);
            _vertexBuffer.push_back(vertex.z);

            // Retrieve the texture coordinates corresponding to vertices index found
            // in face.
            const Vector2 &texCoord = _textureCoordinates[textureIndex];
            _vertexBuffer.push_back(texCoord.u);
            _vertexBuffer.push_back(texCoord.v);

            // Store the index of the new vertex by calculating last vertex index.
            // Since a contain 5 float, we divide per 5 to know how many vertex are in
            // there.
            // - 1 for finding the new index.
            _uniqueVertices[key] = _vertexBuffer.size() / 5 - 1;
        }

        // Store the new indices that will be used in Vertex Buffer.
        _verticesIndices.push_back(_uniqueVertices[key]);
    }
}

void Model::triangulateFaces(const std::vector<std::string> &verticesString) {

    for (unsigned int i = 1; i < verticesString.size() - 1; ++i) {
        _triangleVertices.clear();

        // emplace_back is more memory-efficient than push_back because it moves the
        // object instead of copying it. https://www.youtube.com/watch?v=HcESuwmlHEY

        _triangleVertices.emplace_back(verticesString[0]);
        _triangleVertices.emplace_back(verticesString[i]);
        _triangleVertices.emplace_back(verticesString[i + 1]);

        createFaces(_triangleVertices);
    }
}

void Model::parse(const std::string &filename) {
    if (filename.find(".obj") == std::string::npos) {
        throw std::runtime_error("Error: file is not an obj.");
    }

    std::ifstream infile(filename);

    if (!infile.is_open()) {
        const std::string error = "Error: Cannot open file: " + filename;
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
                _vertices.push_back(vec3);

            } else {
                // Subtract 1 to adapt the texture origin to the top-left corner.
                const Vector2 vec2(values[0], 1.0 - values[1]);
                _textureCoordinates.push_back(vec2);
            }
        }

        else if (type == "f") {
            // At this state, we have parsed all vertices and texture coordinates. So
            // if the container is empty, it means we should calculate texture
            // coordinates.
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
            if (vertices.size() == 3) {
                createFaces(vertices);
            }

            // Apply fan triangulation for polygons with more than 3 vertices.
            else if (vertices.size() > 3) {
                triangulateFaces(vertices);
            }
        }
    }

    calculateCentroid();
}