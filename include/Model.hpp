#pragma once

#include "../include/vector.hpp"
#include <iostream>
#include <vector>

class Model {
    public:
    enum Axis {
        X = 0,
        Y = 1,
        Z = 2,
    };

    Model();
    void parse(const std::string &filename);
    void calculateCentroid();
    void calculateTextureCoordinates();
    void createCombinedVertexBuffer();

    // o
    std::string _name;
    // v
    std::vector<Vector3> _vertex;
    // vn
    std::vector<Vector3> _vertexNormals;
    // vt
    std::vector<Vector2> _textureCoordinates;
    // f
    std::vector<unsigned int> _vertexIndices;
    std::vector<unsigned int> _textureCoordinatesIndices;
    std::vector<unsigned int> _vertexNormalsIndices;
    // s
    std::vector<int> _smoothingGroups;
    // usemtl
    std::vector<std::string> _materials;

    std::vector<float> _combinedVertexBuffer;

    Vector3 _centroid;
};