#pragma once

#include "../include/vector.hpp"
#include <iostream>
#include <vector>

class Model {
    public:
    Model();
    void parse(const std::string &filename);
    void calculateCentroid();
    void calculateTextureCoordinates();

    // v
    std::vector<Vector3> _vertex;
    // vt
    std::vector<Vector2> _textureCoordinates;
    // f
    std::vector<unsigned int> _vertexIndices;
    std::vector<unsigned int> _textureCoordinatesIndices;

    std::vector<float> _combinedVertexBuffer;

    Vector3 _centroid;
};