#pragma once

#include "math/vector.hpp"
#include "Vertex.hpp"
#include <string>
#include <cstdint>
#include <unordered_map>
#include <vector>

class Mesh
{
    public:
    Mesh(const std::string& filename);

    void TriangulateFaces(const std::vector<std::string>& verticesString);

    void CreateFaces(const std::vector<std::string>& vertices);
    void CalculatePivot();
    void CalculateTextureCoordinates();

    std::vector<Vector3> _vertices;
    std::vector<uint32_t> _verticesIndices;

    std::vector<Vector3> _normals;

    std::vector<Vector2> _textureCoordinates;
    std::vector<uint32_t> _textureIndices;

    std::vector<Vertex> _vertexBuffer;

    std::unordered_map<std::string, uint32_t> _uniqueVertices;
    std::vector<std::string> _triangleVertices;

    Vector3 _pivot;

    private:
    std::vector<bool> _hasNormal;
    std::vector<bool> _hasTexCoord;
};