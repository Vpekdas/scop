#include "../include/Model.hpp"
#include "../include/Render.hpp"
#include "../include/colors.hpp"
#include <iostream>

int main(int ac, char **av) {

    if (ac != 2) {
        std::cerr << NEON_RED << "Error: Usage is <scop> <.obj>" << RESET << std::endl;
        return 1;
    }
    try {

        // Render Render;
        // Render.init();

        Model Model;
        Model.parse(av[1]);

        bool running = true;

        while (running) {
            // Events
            SDL_Event event;

            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED || event.type == SDL_EVENT_QUIT) {
                    running = false;
                }
            }

            // Update
            // Render
        }
    } catch (std::exception &ex) {
        std::cerr << NEON_RED << ex.what() << RESET << std::endl;
    }
}
