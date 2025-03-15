#include "../include/Model.hpp"
#include "../include/Renderer.hpp"
#include "../include/colors.hpp"
#include <iostream>

int main(int ac, char **av) {

    if (ac != 2) {
        std::cerr << NEON_RED << "Error: Usage is <scop> <.obj>" << RESET << std::endl;
        return 1;
    }
    try {

        Renderer Renderer(av[1]);

        Renderer.mainLoop();

    } catch (std::exception &ex) {
        std::cerr << NEON_RED << ex.what() << RESET << std::endl;
    }
}
