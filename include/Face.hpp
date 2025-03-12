#pragma once

#include <vector>

class Face {
    public:
    Face();
    Face(const std::vector<int> &vertexIndices, const std::vector<int> &textureIndices,
         const std::vector<int> &normalIndices);
    ~Face();

    const std::vector<int> &getVertexIndices() const;
    const std::vector<int> &getTextureIndices() const;
    const std::vector<int> &getNormalIndices() const;

    private:
    std::vector<int> _vertexIndices;
    std::vector<int> _textureIndices;
    std::vector<int> _normalIndices;

    protected:
};

std::ostream &operator<<(std::ostream &out, const Face &face);
