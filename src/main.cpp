#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include <SDL2/SDL_video.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>

#include <glad/glad.h>
#include <SDL3/SDL_opengl.h>

int main()
{
    SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);

    // Initialize
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_Window* window = SDL_CreateWindow("scop", 1280, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);

    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    bool running = true;
    
    while (running)
    {
        // Events
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED || event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        // Update
        // Render
    }

    SDL_DestroyWindow(window);
    SDL_GL_DestroyContext(gl_context);
    SDL_Quit();
}
