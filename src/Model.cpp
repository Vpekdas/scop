#include "../include/Model.hpp"
#include <fstream>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

Model::Model()
    : m_vertices(), m_textureCoordinates(), m_verticesIndices(), m_textureIndices(), m_vertexBuffer(),
      _centroid(0, 0, 0) {
}

void Model::calculateCentroid() {
    if (m_vertices.empty()) {
        _centroid = Vector3(0, 0, 0);
        return;
    }

    Vector3 sum =
        std::accumulate(m_vertices.begin(), m_vertices.end(), Vector3(0, 0, 0),
                        [](const Vector3 &a, const Vector3 &b) { return Vector3(a.x + b.x, a.y + b.y, a.z + b.z); });

    _centroid = Vector3(sum.x / m_vertices.size(), sum.y / m_vertices.size(), sum.z / m_vertices.size());
}

void Model::calculateTextureCoordinates() {
    if (m_vertices.empty())
        throw std::runtime_error("Error: no vertices are found in obj.");

    // Using bounding box method was creating more stretches than this method.
    // So I'm just creating UV map for each triangles.
    for (size_t i = 0; i < m_vertices.size(); i += 3) {
        Vector3 vertex0 = m_vertices[i];
        Vector3 vertex1 = m_vertices[i + 1];
        Vector3 vertex2 = m_vertices[i + 2];

        Vector2 uv0(vertex0.x, vertex0.y);
        Vector2 uv1(vertex1.x, vertex1.y);
        Vector2 uv2(vertex2.x, vertex2.y);

        m_textureCoordinates.push_back(uv0);
        m_textureCoordinates.push_back(uv1);
        m_textureCoordinates.push_back(uv2);
    }

    for (size_t i = 0; i < m_vertices.size(); i++) {
        m_textureIndices.push_back(i);
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
                m_vertices.push_back(vec3);

            } else if (type == "vt") {
                // Subtract 1 to adapt the texture origin to the top-left corner.
                const Vector2 vec2(values[0], 1.0 - values[1]);
                m_textureCoordinates.push_back(vec2);
            }
        }

        else if (type == "f") {
            // At this state, we have parsed all vertices and texture coordinates. So if the container is empty, it
            // means we should calculate them.
            if (m_textureCoordinates.empty()) {
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
                    } else {
                        // Ensure that the indices exist.
                        // Protect the case that face is parsed but indices is not in file.
                        if (vertexIndex >= m_textureIndices.size()) {
                            throw std::runtime_error(
                                "Error: Vertex index out of bounds while accessing texture indices.");
                        }
                        textureIndex = m_textureIndices[vertexIndex];
                    }

                    // Generate a unique key for the vertex-texture combination.
                    std::string key = std::to_string(vertexIndex) + "/" + std::to_string(textureIndex);

                    // Check if the vertex-texture combination is not already in the map.
                    if (uniqueVertices.find(key) == uniqueVertices.end()) {

                        // Retrieve the vertices corresponding to vertices index found in face.
                        const Vector3 &vertex = m_vertices[vertexIndex];
                        m_vertexBuffer.push_back(vertex.x);
                        m_vertexBuffer.push_back(vertex.y);
                        m_vertexBuffer.push_back(vertex.z);

                        // Retrieve the texture coordinates corresponding to vertices index found in face.
                        const Vector2 &texCoord = m_textureCoordinates[textureIndex];
                        m_vertexBuffer.push_back(texCoord.u);
                        m_vertexBuffer.push_back(texCoord.v);

                        // Store the index of the new vertex by calculating last vertex index.
                        // Since a contain  5 float, we divide per 5 to know how many vertex are in there.
                        // - 1 for finding the new index.
                        uniqueVertices[key] = m_vertexBuffer.size() / 5 - 1;
                    }

                    // Store the new indices that will be used in Vertex Buffer.
                    m_verticesIndices.push_back(uniqueVertices[key]);
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
                        } else {
                            // Ensure that the indices exist.
                            // Protect the case that face is parsed but indices is not in file.
                            if (vertexIndex >= m_textureIndices.size()) {
                                throw std::runtime_error(
                                    "Error: Vertex index out of bounds while accessing texture indices.");
                            }
                            textureIndex = m_textureIndices[vertexIndex];
                        }

                        // Generate a unique key for the vertex-texture combination.
                        std::string key = std::to_string(vertexIndex) + "/" + std::to_string(textureIndex);

                        // Check if the vertex-texture combination is not already in the map.
                        if (uniqueVertices.find(key) == uniqueVertices.end()) {

                            // Retrieve the vertices corresponding to vertices index found in face.
                            const Vector3 &vertex = m_vertices[vertexIndex];
                            m_vertexBuffer.push_back(vertex.x);
                            m_vertexBuffer.push_back(vertex.y);
                            m_vertexBuffer.push_back(vertex.z);

                            // Retrieve the texture coordinates corresponding to vertices index found in face.
                            const Vector2 &texCoord = m_textureCoordinates[textureIndex];
                            m_vertexBuffer.push_back(texCoord.u);
                            m_vertexBuffer.push_back(texCoord.v);

                            // Store the index of the new vertex by calculating last vertex index.
                            // Since a contain  5 float, we divide per 5 to know how many vertex are in there.
                            // - 1 for finding the new index.
                            uniqueVertices[key] = m_vertexBuffer.size() / 5 - 1;
                        }

                        // Store the new indices that will be used in Vertex Buffer.
                        m_verticesIndices.push_back(uniqueVertices[key]);
                    }
                }
            }
        }
    }
}