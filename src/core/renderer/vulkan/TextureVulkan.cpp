#include "renderer/vulkan/TextureVulkan.hpp"
#include <fstream>
#include <stdexcept>
#include <vector>

TextureVulkan::TextureVulkan(const std::string& path)
    : _filePath(path), _localBuffer(nullptr), _width(0), _height(0), _BPP(0)
{
    LoadTGA(path);
}

TextureVulkan::~TextureVulkan()
{
}

void TextureVulkan::Bind(unsigned int slot) const
{
    (void)slot;
}

void TextureVulkan::Unbind()
{
}

// https://www.ryanjuckett.com/parsing-colors-in-a-tga-file/
// https://www.gamers.org/dEngine/quake3/TGA.txt
void TextureVulkan::LoadTGA(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Error: unable to load TGA File!");
    }

    unsigned char length = 0;
    unsigned char imageType = 0;
    unsigned char bits = 0;
    unsigned short width = 0, height = 0;

    file.read(reinterpret_cast<char*>(&length), sizeof(length));
    file.seekg(1, std::ios::cur);
    file.read(reinterpret_cast<char*>(&imageType), sizeof(imageType));
    file.seekg(9, std::ios::cur);
    file.read(reinterpret_cast<char*>(&width), sizeof(width));
    file.read(reinterpret_cast<char*>(&height), sizeof(height));
    file.read(reinterpret_cast<char*>(&bits), sizeof(bits));
    file.seekg(length + 1, std::ios::cur);

    if (width == 0 || height == 0 || (bits != 24 && bits != 32 && bits != 16))
    {
        throw std::runtime_error("Error: Invalid TGA file: Unsupported dimensions or pixel format.");
    }

    int channels = bits / 8;
    int stride = channels * width;
    const size_t decodedChannels = bits == 16 ? 3u : static_cast<size_t>(channels);
    _localBuffer = std::make_unique<unsigned char[]>(static_cast<size_t>(width) * height * decodedChannels);

    if (imageType != 10)
    { // Not RLE compressed
        if (bits == 24 || bits == 32)
        {
            for (int y = 0; y < height; ++y)
            {
                unsigned char* pLine = &_localBuffer[stride * y];
                file.read(reinterpret_cast<char*>(pLine), stride);
                for (int i = 0; i < stride; i += channels)
                {
                    // Swap because TGA store in BGR order.
                    std::swap(pLine[i], pLine[i + 2]);
                }
            }
        }
        else if (bits == 16)
        {
            unsigned short pixels = 0;
            int r = 0, g = 0, b = 0;
            channels = 3;
            stride = channels * width;
            for (int i = 0; i < width * height; ++i)
            {
                file.read(reinterpret_cast<char*>(&pixels), sizeof(pixels));
                b = (pixels & 0x1f) << 3;
                g = ((pixels >> 5) & 0x1f) << 3;
                r = ((pixels >> 10) & 0x1f) << 3;
                _localBuffer[i * 3 + 0] = r;
                _localBuffer[i * 3 + 1] = g;
                _localBuffer[i * 3 + 2] = b;
            }
        }
        else
        {
            throw std::runtime_error("Error: Unsupported pixel format!");
        }
    }
    else
    { // RLE compressed
        unsigned char rleID = 0;
        int colorsRead = 0;
        std::vector<unsigned char> pColors(channels);
        while (colorsRead < width * height)
        {
            file.read(reinterpret_cast<char*>(&rleID), sizeof(rleID));
            if (rleID < 128)
            {
                ++rleID;
                while (rleID--)
                {
                    file.read(reinterpret_cast<char*>(pColors.data()), channels);
                    for (int j = 0; j < channels; ++j)
                    {
                        _localBuffer[colorsRead * channels + j] = pColors[j];
                    }
                    if (channels == 4)
                    {
                        _localBuffer[colorsRead * channels + 3] = pColors[3];
                    }
                    ++colorsRead;
                }
            }
            else
            {
                rleID -= 127;
                file.read(reinterpret_cast<char*>(pColors.data()), channels);
                while (rleID--)
                {
                    for (int j = 0; j < channels; ++j)
                    {
                        _localBuffer[colorsRead * channels + j] = pColors[j];
                    }
                    if (channels == 4)
                    {
                        _localBuffer[colorsRead * channels + 3] = pColors[3];
                    }
                    ++colorsRead;
                }
            }
        }
    }

    _width = width;
    _height = height;
    _BPP = bits;
    ConvertToRGBA();
}

void TextureVulkan::ConvertToRGBA()
{
    if (_BPP == 32)
    {
        return;
    }

    const size_t pixelCount = static_cast<size_t>(_width) * _height;
    auto rgba = std::make_unique<unsigned char[]>(pixelCount * 4);
    for (size_t i = 0; i < pixelCount; ++i)
    {
        rgba[i * 4] = _localBuffer[i * 3];
        rgba[i * 4 + 1] = _localBuffer[i * 3 + 1];
        rgba[i * 4 + 2] = _localBuffer[i * 3 + 2];
        rgba[i * 4 + 3] = 255;
    }
    _localBuffer = std::move(rgba);
    _BPP = 32;
}
