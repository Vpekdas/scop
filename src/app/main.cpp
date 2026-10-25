#include <cstdio>
#include <print>
#include "app/Application.hpp"
#include <filesystem>

int main(int ac, char** av)
{
    if (ac != 3)
    {
        std::println(stderr, "Error: Usage is <scop> <.obj> <texture.tga>");
        return 1;
    }

    try
    {
        Application app(1280, 720, "scop");

        app.LoadMesh(std::filesystem::path(av[1]));
        app.LoadTexture(std::filesystem::path(av[2]));
        app.LoadNoiseTexture(std::filesystem::path(ASSET_DIR) / "textures" / "solidnoise.tga");

        app.Run();
    }
    catch (std::exception& ex)
    {
        std::println(stderr, "{}", ex.what());
    }
}