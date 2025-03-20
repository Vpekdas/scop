#include "../include/Renderer.hpp"
#include "../include/colors.hpp"
#include <iostream>

int main(int ac, char **av) {

    if (ac != 3) {
        std::cerr << NEON_RED << "Error: Usage is <scop> <.obj> <texture.tga>" << RESET << std::endl;
        return 1;
    }
    try {

        Renderer Renderer(av[1], av[2]);

        Renderer.mainLoop();

    } catch (std::exception &ex) {
        std::cerr << NEON_RED << ex.what() << RESET << std::endl;
    }
}
