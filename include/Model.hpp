#pragma once

#include "../include/vector.hpp"
#include <vector>

class Model {
    public:
    Model(const std::string &filename);

    void triangulateFaces(const std::vector<std::string> &verticesString);

    void createFaces(const std::vector<std::string> &vertices);

    // Calculate the centroid of the model by summing the x, y, and z coordinates
    // of all vertices and dividing by the total number of vertices.
    void calculateCentroid();

    // Generate texture coordinates and their indices based model size.
    // The generation is performed for each triangle.
    void calculateTextureCoordinates();

    std::vector<Vector3> _vertices;
    std::vector<unsigned int> _verticesIndices;

    std::vector<Vector2> _textureCoordinates;
    std::vector<unsigned int> _textureIndices;

    std::vector<float> _vertexBuffer;

    std::unordered_map<std::string, unsigned int> _uniqueVertices;
    std::vector<std::string> _triangleVertices;

    Vector3 _centroid;
};