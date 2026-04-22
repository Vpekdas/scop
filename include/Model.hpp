#pragma once

#include "math/vector.hpp"
#include <cstdint>
#include <vector>

class Model
{
    public:
    Model(const std::string& filename);

    void TriangulateFaces(const std::vector<std::string>& verticesString);

    void CreateFaces(const std::vector<std::string>& vertices);
    void CalculateCentroid();
    void CalculateTextureCoordinates();

    std::vector<Vector3> _vertices;
    std::vector<uint32_t> _verticesIndices;

    std::vector<Vector2> _textureCoordinates;
    std::vector<uint32_t> _textureIndices;

    std::vector<float> _vertexBuffer;

    std::unordered_map<std::string, uint32_t> _uniqueVertices;
    std::vector<std::string> _triangleVertices;

    Vector3 _centroid;
};