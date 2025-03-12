#pragma once

#include <array>

class Face {
    public:
    Face();
    Face(const std::array<int, 3> &vertexIndices, const std::array<int, 3> &textureIndices,
         const std::array<int, 3> &normalIndices);
    ~Face();

    const std::array<int, 3> &getVertexIndices() const;
    const std::array<int, 3> &getTextureIndices() const;
    const std::array<int, 3> &getNormalIndices() const;

    private:
    std::array<int, 3> _vertexIndices;
    std::array<int, 3> _textureIndices;
    std::array<int, 3> _normalIndices;

    protected:
};