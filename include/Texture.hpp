#pragma once

#include "../include/Renderer.hpp"
#include <memory>

struct tImageTGA {
    std::vector<unsigned char> data;
    int channels;
    int sizeX;
    int sizeY;
};

class Texture {
    public:
    Texture(const std::string &path);
    ~Texture();

    void Bind(unsigned int slot = 0) const;
    void Unbind();

    // https://nehe.gamedev.net/tutorial/loading_compressed_and_uncompressed_tga%27s/22001/
    // https://github.com/gamedev-net/nehe-opengl/blob/master/glut/lesson22/lesson22_glut/src/tga.cpp
    void LoadTGA(const std::string &filename);

    private:
    unsigned int m_RendererID;
    std::string m_FilePath;
    std::unique_ptr<unsigned char[]> m_LocalBuffer;
    int m_Width, m_Height, m_BPP;

    inline int GetWidth() const {
        return m_Width;
    }

    inline int GetHeight() const {
        return m_Height;
    }
};