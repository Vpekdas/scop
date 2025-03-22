#pragma once

#include "../include/vector.hpp"
#include <iostream>
#include <vector>

class Model {
    public:
    Model();

    // Open the .obj file and retrieve all the necessary information to create vertices.
    // This includes parsing vertex positions (v), texture coordinates (vt), and face indices (f).
    void parse(const std::string &filename);

    // Calculate the centroid of the model by summing the x, y, and z coordinates of all vertices
    // and dividing by the total number of vertices.
    void calculateCentroid();

    // Generate texture coordinates and their indices based on vertex positions.
    // The generation is performed for each triangle.
    void calculateTextureCoordinates();

    // v
    std::vector<Vector3> m_vertices;
    // vt
    std::vector<Vector2> m_textureCoordinates;
    // f
    std::vector<unsigned int> m_verticesIndices;
    std::vector<unsigned int> m_textureIndices;

    std::vector<float> m_vertexBuffer;

    Vector3 _centroid;
};