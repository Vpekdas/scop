#include "app/Application.hpp"
#include <memory>

#ifdef USE_OPENGL
#include "renderer/opengl/RendererOpenGL.hpp"
#endif

#ifdef USE_VULKAN
#include "renderer/vulkan/RendererVulkan.hpp"
#endif

Application::Application(uint32_t width, uint32_t height, const char* title) : _camera(0.0f, 0.0f, 10.0f)
{
    if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO))
    {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_WindowFlags flags = 0;

#ifdef USE_OPENGL

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    flags = SDL_WINDOW_OPENGL;

#elif defined(USE_VULKAN)
    flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;

#endif

    _window = std::make_unique<Window>(width, height, title, flags);

#ifdef USE_OPENGL
    _renderer = std::make_unique<RendererOpenGL>(*_window);
#endif
#ifdef USE_VULKAN
    _renderer = std::make_unique<RendererVulkan>(*_window);
#endif
}

Application::~Application()
{
}

void Application::Run()
{

    Uint64 lastTime = SDL_GetPerformanceCounter();

    _renderer->Start();

    while (_isRunning)

    {
        Uint64 currentTime = SDL_GetPerformanceCounter();
        SDL_Event event;

        float deltaTime = (currentTime - lastTime) / (float)SDL_GetPerformanceFrequency();
        lastTime = currentTime;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED || event.type == SDL_EVENT_QUIT)
            {
                _isRunning = false;
            }
            else if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED &&
                     event.window.windowID == SDL_GetWindowID(_window->GetSDLWindow()))
            {
                _renderer->OnWindowResized();
            }
            else if (event.type == SDL_EVENT_KEY_DOWN)
            {
                switch (event.key.key)
                {
                case SDLK_ESCAPE:
                    _isRunning = false;
                    break;
                case SDLK_F1:
                    _renderer->SetPolygonMode(RenderMode::POINT);
                    break;
                case SDLK_F2:
                    _renderer->SetPolygonMode(RenderMode::LINE);
                    break;
                case SDLK_F3:
                    _renderer->SetPolygonMode(RenderMode::FILL);
                    break;
                case SDLK_F4:
                    _renderer->ApplyTexture();
                    break;
                case SDLK_F5:
                    _renderer->ApplyDissolve();
                    break;
                case SDLK_X:
                    _renderer->SetRotationAxis(RotationAxis::X);
                    break;
                case SDLK_Y:
                    _renderer->SetRotationAxis(RotationAxis::Y);
                    break;
                case SDLK_Z:
                    _renderer->SetRotationAxis(RotationAxis::Z);
                    break;
                case SDLK_SPACE:
                    _renderer->SetRotationAxis(RotationAxis::NONE);
                    break;
                }
            }
        }

        _camera.Move(deltaTime);
        _renderer->Update(deltaTime, _camera);
        _renderer->Render();
        _renderer->SwapBuffers();
    }
}
