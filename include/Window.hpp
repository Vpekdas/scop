#pragma once
#include "SDL3/SDL_video.h"

class Window
{
    public:
    Window() = delete;
    explicit Window(uint32_t width, uint32_t height, const char* title, SDL_WindowFlags flags);
    ~Window();

    inline uint32_t GetWindowWidth() const
    {
        return _width;
    }

    inline uint32_t GetWindowHeight() const
    {
        return _height;
    }

    inline SDL_Window* GetSDLWindow()
    {
        return _window;
    }

    private:
    SDL_Window* _window;

    uint32_t _width;
    uint32_t _height;
};
