#pragma once

#include "../include/Face.hpp"
#include "../include/Vector2.hpp"
#include "../include/Vector3.hpp"
#include <vector>
#include <iostream>

class Model {
    public:
    Model();
    ~Model();
    void parse(const std::string &filename);

    const std::string &getName() const;
    const std::vector<Vector3> &getVertex() const;
    const std::vector<Vector3> &getVertexNormals() const;
    const std::vector<Vector2> &getTextureCoordinates() const;
    const std::vector<Face> &getFaces() const;
    const std::vector<int> &getSmoothingGroups() const;
    const std::vector<std::string> &getMaterials() const;

    private:
    // o
    std::string _name;
    // v
    std::vector<Vector3> _vertex;
    // vn
    std::vector<Vector3> _vertexNormals;
    // vt
    std::vector<Vector2> _textureCoordinates;
    // f
    std::vector<Face> _faces;
    // s
    std::vector<int> _smoothingGroups;
    // usemtl
    std::vector<std::string> _materials;

    protected:
};