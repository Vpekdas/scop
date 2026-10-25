#include "Mesh.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

Mesh::Mesh(const std::string& filename)
    : _vertices(), _verticesIndices(), _textureCoordinates(), _textureIndices(), _vertexBuffer(), _uniqueVertices(),
      _triangleVertices(), _pivot(0, 0, 0)
{

    _triangleVertices.reserve(3);

    if (filename.find(".obj") == std::string::npos)
    {
        throw std::runtime_error("Error: file is not an obj.");
    }

    std::ifstream infile(filename);

    if (!infile.is_open())
    {
        const std::string error = "Error: Cannot open file: " + filename;
        throw std::runtime_error(error);
    }

    std::string line, type;
    std::size_t start = 0;

    while (std::getline(infile, line))
    {
        start = line.find(" ");
        type = line.substr(0, start);

        if (type == "v" || type == "vt" || type == "vn")
        {
            std::istringstream iss(line.substr(start + 1));
            std::vector<float> values;
            float value;

            while (iss >> value)
            {
                values.push_back(value);
            }

            if (values.size() < (type == "vt" ? 2u : 3u))
            {
                throw std::runtime_error("Error: Invalid OBJ vertex attribute");
            }

            if (type == "vn")
            {
                _normals.emplace_back(values[0], values[1], values[2]);
            }
            else if (type == "v")
            {
                const Vector3 vec3(values[0], values[1], values[2]);
                _vertices.push_back(vec3);
            }
            else
            {
                // Subtract 1 to adapt the texture origin to the top-left corner.
                const Vector2 vec2(values[0], 1.0 - values[1]);
                _textureCoordinates.push_back(vec2);
            }
        }

        else if (type == "f")
        {
            std::istringstream iss(line.substr(start + 1));
            std::vector<std::string> vertices;
            std::string vertex;

            // Split each vertex in a string.
            // Ex: f 1/1/1 5/2/1 7/3/1 3/4/1 -> vertices[0] = 1/1/1.
            while (iss >> vertex)
            {
                vertices.push_back(vertex);
            }
            if (vertices.size() == 3)
            {
                CreateFaces(vertices);
            }

            else if (vertices.size() > 3)
            {
                TriangulateFaces(vertices);
            }
        }
    }

    // Accumulate area-weighted normals for the indexed mesh.
    // Triangle cross product: https://wikis.khronos.org/opengl/Calculating_a_Surface_Normal
    for (size_t i = 0; i + 2 < _verticesIndices.size(); i += 3)
    {
        auto& a = _vertexBuffer[_verticesIndices[i]];
        auto& b = _vertexBuffer[_verticesIndices[i + 1]];
        auto& c = _vertexBuffer[_verticesIndices[i + 2]];
        const Vector3 ab(b.position.x - a.position.x, b.position.y - a.position.y, b.position.z - a.position.z);
        const Vector3 ac(c.position.x - a.position.x, c.position.y - a.position.y, c.position.z - a.position.z);
        const Vector3 normal(ab.y * ac.z - ab.z * ac.y, ab.z * ac.x - ab.x * ac.z, ab.x * ac.y - ab.y * ac.x);
        for (size_t j = i; j < i + 3; ++j)
        {
            const auto index = _verticesIndices[j];
            if (!_hasNormal[index])
            {
                auto& generated = _vertexBuffer[index].normal;
                generated.x += normal.x;
                generated.y += normal.y;
                generated.z += normal.z;
            }
        }
    }
    for (size_t i = 0; i < _vertexBuffer.size(); ++i)
    {
        if (_hasNormal[i])
        {
            continue;
        }
        auto& normal = _vertexBuffer[i].normal;
        const float length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
        if (length > 0.0f)
        {
            normal = Vector3(normal.x / length, normal.y / length, normal.z / length);
        }
    }
    CalculateTextureCoordinates();
    CalculatePivot();
}

void Mesh::CalculatePivot()
{
    if (_vertices.empty())
    {
        _pivot = Vector3(0, 0, 0);
        return;
    }

    Vector3 sum =
        std::accumulate(_vertices.begin(), _vertices.end(), Vector3(0, 0, 0),
                        [](const Vector3& a, const Vector3& b) { return Vector3(a.x + b.x, a.y + b.y, a.z + b.z); });

    _pivot = Vector3(sum.x / _vertices.size(), sum.y / _vertices.size(), sum.z / _vertices.size());
}

// https://community.khronos.org/t/calc-texture-coordinate/14707/5
void Mesh::CalculateTextureCoordinates()
{
    if (_vertices.empty())
    {
        return;
    }

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    for (const auto& position : _vertices)
    {
        minX = std::min(minX, position.x);
        maxX = std::max(maxX, position.x);
        minY = std::min(minY, position.y);
        maxY = std::max(maxY, position.y);
    }

    for (size_t i = 0; i < _vertexBuffer.size(); ++i)
    {
        if (_hasTexCoord[i])
        {
            continue;
        }
        const auto& position = _vertexBuffer[i].position;
        const float u = maxX > minX ? (position.x - minX) / (maxX - minX) : 0.0f;
        const float v = maxY > minY ? (position.y - minY) / (maxY - minY) : 0.0f;
        _vertexBuffer[i].texCoord = Vector2(u, v);
    }
}

void Mesh::CreateFaces(const std::vector<std::string>& vertices)
{
    const auto resolveIndex = [](const std::string& text, size_t count) -> uint32_t
    {
        const auto value = std::stoll(text);
        const auto resolved = value > 0 ? value - 1 : static_cast<long long>(count) + value;
        if (value == 0 || resolved < 0 || static_cast<size_t>(resolved) >= count)
        {
            throw std::runtime_error("Error: OBJ index out of bounds");
        }
        return static_cast<uint32_t>(resolved);
    };

    for (const auto& triangleVertex : vertices)
    {
        std::istringstream vertexStream(triangleVertex);
        std::string index;
        unsigned int vertexIndex = 0, textureIndex = 0;

        // Parse vertices indices. (first /)
        if (std::getline(vertexStream, index, '/') && !index.empty())
        {
            vertexIndex = resolveIndex(index, _vertices.size());
        }

        // Missing UVs are generated per face after normals have been calculated.
        const bool hasTexCoord = std::getline(vertexStream, index, '/') && !index.empty();
        if (hasTexCoord)
        {
            textureIndex = resolveIndex(index, _textureCoordinates.size());
        }

        // Third face component references an authored normal (v/vt/vn or v//vn).
        const bool hasNormal = std::getline(vertexStream, index, '/') && !index.empty();
        const uint32_t normalIndex = hasNormal ? resolveIndex(index, _normals.size()) : 0;

        // Distinct normal indices preserve hard edges at shared positions.
        const std::string key = std::to_string(vertexIndex) + "/" +
                                (hasTexCoord ? std::to_string(textureIndex) : "missing") + "/" +
                                (hasNormal ? std::to_string(normalIndex) : "generated");

        if (_uniqueVertices.find(key) == _uniqueVertices.end())
        {

            // Retrieve the vertices corresponding to vertices index found in face.
            const Vector3& vertex = _vertices[vertexIndex];
            const Vector2 texCoord = hasTexCoord ? _textureCoordinates[textureIndex] : Vector2{};
            _vertexBuffer.push_back(Vertex{
                .position = vertex, .normal = hasNormal ? _normals[normalIndex] : Vector3{}, .texCoord = texCoord});
            _hasNormal.push_back(hasNormal);
            _hasTexCoord.push_back(hasTexCoord);
            _uniqueVertices[key] = static_cast<uint32_t>(_vertexBuffer.size() - 1);
        }

        // Store the new indices that will be used in Vertex Buffer.
        _verticesIndices.push_back(_uniqueVertices[key]);
    }
}

void Mesh::TriangulateFaces(const std::vector<std::string>& verticesString)
{

    for (unsigned int i = 1; i < verticesString.size() - 1; ++i)
    {
        _triangleVertices.clear();

        // emplace_back is more memory-efficient than push_back because it moves the
        // object instead of copying it. https://www.youtube.com/watch?v=HcESuwmlHEY
        _triangleVertices.emplace_back(verticesString[0]);
        _triangleVertices.emplace_back(verticesString[i]);
        _triangleVertices.emplace_back(verticesString[i + 1]);

        CreateFaces(_triangleVertices);
    }
}
