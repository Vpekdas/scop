#pragma once

#include "math/Matrix4.hpp"

struct UniformBufferObject
{
    alignas(16) Matrix4 model;
    alignas(16) Matrix4 view;
    alignas(16) Matrix4 proj;
};
