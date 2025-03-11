#pragma once

#include <vector>

class Model {
    public:
    Model();
    ~Model();
    void parse(const std::string &filename);

    const std::string &getName();
    const std::vector<float> &getVertex();
    const std::vector<float> &getVertexNormals();
    const std::vector<float> &getTextureCoordinates();
    const std::vector<int> &getFaces();
    const std::vector<int> &getSmoothingGroups();
    const std::vector<std::string> &getMaterials();

    private:
    // o
    std::string _name;
    // v
    std::vector<float> _vertex;
    // vn
    std::vector<float> _vertexNormals;
    // vt
    std::vector<float> _textureCoordinates;
    // f
    std::vector<int> _faces;
    // s
    std::vector<int> _smoothingGroups;
    // usemtl
    std::vector<std::string> _materials;

    protected:
};