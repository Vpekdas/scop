#pragma once

#include "ITexture.hpp"
#include <cstdint>
#include <memory>
#include <vector>

struct tImageTGA
{
    std::vector<unsigned char> data;
    int channels;
    int sizeX;
    int sizeY;
};

class TextureOpenGL : public ITexture
{
    public:
    TextureOpenGL(const std::string& path);
    ~TextureOpenGL();

    void Bind(unsigned int slot = 0) const override;
    void Unbind();
    virtual inline uint32_t GetWidth() const override
    {
        return _width;
    }

    virtual inline uint32_t GetHeight() const override
    {
        return _height;
    }

    // https://nehe.gamedev.net/tutorial/loading_compressed_and_uncompressed_tga%27s/22001/
    // https://github.com/gamedev-net/nehe-opengl/blob/master/glut/lesson22/lesson22_glut/src/tga.cpp
    void LoadTGA(const std::string& filename);

    private:
    unsigned int _rendererID;
    std::string _filePath;
    std::unique_ptr<unsigned char[]> _localBuffer;
    uint32_t _width;
    uint32_t _height;
    uint32_t _BPP;
};