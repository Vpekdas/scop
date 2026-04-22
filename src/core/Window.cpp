#include "Window.hpp"
#include <stdexcept>

Window::Window(uint32_t width, uint32_t height, const char* title, SDL_WindowFlags flags)
    : _width(width), _height(height)
{
    _window = SDL_CreateWindow(title, width, height, flags);

    if (!_window)
    {
        throw std::runtime_error(SDL_GetError());
    }
}

Window::~Window()
{
    SDL_DestroyWindow(_window);
}
