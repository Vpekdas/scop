#pragma once

#include "../include/vector.hpp"
#include <iostream>
#include <vector>

class Model {
    public:
    Model();
    void parse(const std::string &filename);

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
};