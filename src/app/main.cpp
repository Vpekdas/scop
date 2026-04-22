#include "app/Application.hpp"
#include <filesystem>

int main(int ac, char** av)
{
    if (ac != 3)
    {
        std::cerr << "Error: Usage is <scop> <.obj> <texture.tga>" << "\n";
        return 1;
    }

    try
    {
        Application app(1280, 720, "scop");

        app.LoadModel(std::filesystem::path(av[1]));
        app.LoadTexture(std::filesystem::path(av[2]));
        app.LoadNoiseTexture(std::filesystem::path(ASSET_DIR) / "textures" / "solidnoise.tga");

        app.Run();
    }
    catch (std::exception& ex)
    {
        std::cerr << ex.what() << "\n";
    }
}