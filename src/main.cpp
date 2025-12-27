#include "../include/Renderer.hpp"
#include <filesystem>
#include <memory>

int main(int ac, char **av) {

    (void)av;

    if (ac != 3) {
        std::cerr << "Error: Usage is <scop> <.obj> <texture.tga>" << std::endl;
        return 1;
    }
    try {
        Renderer Renderer;
        auto model = std::make_unique<Model>(av[1]);
        Renderer.SetModel(std::move(model));

        auto texture = std::make_unique<Texture>(av[2]);

        std::filesystem::path noisePath = std::filesystem::current_path() / "assets" / "textures" / "solidnoise.tga";
        auto noiseTexture = std::make_unique<Texture>(noisePath);

        Renderer.SetTexture(std::move(texture));
        Renderer.SetNoiseTexture(std::move(noiseTexture));

        Renderer.Loop();

    } catch (std::exception &ex) {
        std::cerr << ex.what() << std::endl;
    }
}
