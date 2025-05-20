#include "../include/Renderer.hpp"
#include <iostream>

int main(int ac, char **av) {

    if (ac != 3) {
        std::cerr << "Error: Usage is <scop> <.obj> <texture.tga>" << std::endl;
        return 1;
    }
    try {
        Renderer Renderer(av[1], av[2]);
        Renderer.mainLoop();

    } catch (std::exception &ex) {
        std::cerr << ex.what() << std::endl;
    }
}
