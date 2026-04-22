#include "app/Application.hpp"
#include "AudioPlayer.hpp"
#include <memory>

#ifdef USE_OPENGL
#include "renderer/opengl/RendererOpenGL.hpp"
#endif

Application::Application(uint32_t width, uint32_t height, const char* title) : _camera(0.0f, 0.0f, 10.0f)
{
    if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_AUDIO))
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

#endif

    _window = std::make_unique<Window>(width, height, title, flags);

    std::filesystem::path wawPath = std::filesystem::path(ASSET_DIR) / "sounds" / "bad_apple.wav";
    _audioPlayer = std::make_unique<AudioPlayer>(wawPath);

#ifdef USE_OPENGL
    _renderer = std::make_unique<RendererOpenGL>(*_window);
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
                case SDLK_F6:
                    _renderer->PlayBadApple();
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
                case SDLK_P:
                    _audioPlayer->GetIsPlaying() ? _audioPlayer->Stop() : _audioPlayer->Play();
                    break;
                }
            }
        }

        size_t frame = 0;

        if (_audioPlayer->GetIsPlaying())
        {
            double time = _audioPlayer->GetPlaybackTime();
            frame = static_cast<size_t>(time * BAD_APPLE_FPS);
        }

        _renderer->SetFrame(frame);

        _camera.Move(deltaTime);
        _renderer->Update(deltaTime, _camera);
        _renderer->Render();
        _renderer->SwapBuffers();
    }
}
