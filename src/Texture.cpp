#include "../include/Texture.hpp"
#include <fstream>
#include <stdexcept>
#include <vector>

Texture::Texture(const std::string &path)
    : m_RendererID(0), m_FilePath(path), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0) {
    LoadTGA(path);

    GlCall(glGenTextures(1, &m_RendererID));
    GlCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));

    GlCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GlCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GlCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GlCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    GLenum format = (m_BPP == 24) ? GL_RGB : GL_RGBA;

    GlCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, m_LocalBuffer));

    GlCall(glBindTexture(GL_TEXTURE_2D, 0));
}

Texture::~Texture() {
    if (m_LocalBuffer) {
        delete[] m_LocalBuffer;
    }
}

void Texture::Bind(unsigned int slot) const {
    GlCall(glActiveTexture(GL_TEXTURE0 + slot));
    GlCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
}

void Texture::Unbind() {
}

void Texture::LoadTGA(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Unable to load TGA File!");
    }

    unsigned char length = 0;
    unsigned char imageType = 0;
    unsigned char bits = 0;
    unsigned short width = 0, height = 0;

    file.read(reinterpret_cast<char *>(&length), sizeof(length));
    file.seekg(1, std::ios::cur);
    file.read(reinterpret_cast<char *>(&imageType), sizeof(imageType));
    file.seekg(9, std::ios::cur);
    file.read(reinterpret_cast<char *>(&width), sizeof(width));
    file.read(reinterpret_cast<char *>(&height), sizeof(height));
    file.read(reinterpret_cast<char *>(&bits), sizeof(bits));
    file.seekg(length + 1, std::ios::cur);

    int channels = bits / 8;
    int stride = channels * width;
    m_LocalBuffer = new unsigned char[stride * height];

    std::cout << "TGA File Info: " << std::endl;
    std::cout << "Image Type: " << static_cast<int>(imageType) << std::endl;
    std::cout << "Bits per Pixel: " << static_cast<int>(bits) << std::endl;
    std::cout << "Width: " << width << std::endl;
    std::cout << "Height: " << height << std::endl;

    if (imageType != 10) { // Not RLE compressed
        if (bits == 24 || bits == 32) {
            for (int y = 0; y < height; ++y) {
                unsigned char *pLine = &m_LocalBuffer[stride * y];
                file.read(reinterpret_cast<char *>(pLine), stride);
                for (int i = 0; i < stride; i += channels) {
                    std::swap(pLine[i], pLine[i + 2]);
                }
            }
        } else if (bits == 16) {
            unsigned short pixels = 0;
            int r = 0, g = 0, b = 0;
            channels = 3;
            stride = channels * width;
            delete[] m_LocalBuffer;
            m_LocalBuffer = new unsigned char[stride * height];
            for (int i = 0; i < width * height; ++i) {
                file.read(reinterpret_cast<char *>(&pixels), sizeof(pixels));
                b = (pixels & 0x1f) << 3;
                g = ((pixels >> 5) & 0x1f) << 3;
                r = ((pixels >> 10) & 0x1f) << 3;
                m_LocalBuffer[i * 3 + 0] = r;
                m_LocalBuffer[i * 3 + 1] = g;
                m_LocalBuffer[i * 3 + 2] = b;
            }
        } else {
            throw std::runtime_error("Unsupported pixel format!");
        }
    } else { // RLE compressed
        unsigned char rleID = 0;
        int colorsRead = 0;
        std::vector<unsigned char> pColors(channels);
        while (colorsRead < width * height) {
            file.read(reinterpret_cast<char *>(&rleID), sizeof(rleID));
            if (rleID < 128) {
                ++rleID;
                while (rleID--) {
                    file.read(reinterpret_cast<char *>(pColors.data()), channels);
                    for (int j = 0; j < channels; ++j) {
                        m_LocalBuffer[colorsRead * channels + j] = pColors[j];
                    }
                    if (channels == 4) {
                        m_LocalBuffer[colorsRead * channels + 3] = pColors[3];
                    }
                    ++colorsRead;
                }
            } else {
                rleID -= 127;
                file.read(reinterpret_cast<char *>(pColors.data()), channels);
                while (rleID--) {
                    for (int j = 0; j < channels; ++j) {
                        m_LocalBuffer[colorsRead * channels + j] = pColors[j];
                    }
                    if (channels == 4) {
                        m_LocalBuffer[colorsRead * channels + 3] = pColors[3];
                    }
                    ++colorsRead;
                }
            }
        }
    }

    m_Width = width;
    m_Height = height;
    m_BPP = bits;

    std::cout << "Verifying Pixel Data:" << std::endl;
    for (int y = 0; y < m_Height; ++y) {
        for (int x = 0; x < m_Width; ++x) {
            int index = (y * m_Width + x) * (m_BPP / 8); // Calculate the pixel index
            unsigned char r = m_LocalBuffer[index];
            unsigned char g = m_LocalBuffer[index + 1];
            unsigned char b = m_LocalBuffer[index + 2];
            std::cout << "Pixel (" << x << ", " << y << "): R=" << (int)r << ", G=" << (int)g << ", B=" << (int)b;
            if (m_BPP == 32) { // If the image has an alpha channel
                unsigned char a = m_LocalBuffer[index + 3];
                std::cout << ", A=" << (int)a;
            }
            std::cout << std::endl;
        }
    }
}