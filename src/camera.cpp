#include "camera.hpp"
#include "SDL3/SDL_keyboard.h"
#include "renderer/opengl/RendererOpenGL.hpp"

void Camera::Move(float deltaTime)
{
    const bool* keystates = SDL_GetKeyboardState(nullptr);

    if (keystates[SDL_SCANCODE_W])
    {
        pos.z -= CAMERA_SPEED * deltaTime;
    }
    if (keystates[SDL_SCANCODE_S])
    {
        pos.z += CAMERA_SPEED * deltaTime;
    }
    if (keystates[SDL_SCANCODE_A])
    {
        pos.x -= CAMERA_SPEED * deltaTime;
    }
    if (keystates[SDL_SCANCODE_D])
    {
        pos.x += CAMERA_SPEED * deltaTime;
    }
    if (keystates[SDL_SCANCODE_LEFT])
    {
        rotationAngle += CAMERA_SPEED * deltaTime;
    }
    if (keystates[SDL_SCANCODE_RIGHT])
    {
        rotationAngle -= CAMERA_SPEED * deltaTime;
    }
}