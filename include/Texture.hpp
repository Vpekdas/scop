#pragma once

#include <memory>
#include <vector>

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
    unsigned int _rendererID;
    std::string _filePath;
    std::unique_ptr<unsigned char[]> _localBuffer;
    int _width, _height, _BPP;

    inline int GetWidth() const {
        return _width;
    }

    inline int GetHeight() const {
        return _height;
    }
};