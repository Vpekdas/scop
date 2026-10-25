#pragma once

#include "Vertex.hpp"
#include <array>
#include <cstddef>
#include <vulkan/vulkan.hpp>

inline vk::VertexInputBindingDescription GetVertexBindingDescription()
{
    return {.binding = 0, .stride = sizeof(Vertex), .inputRate = vk::VertexInputRate::eVertex};
}

inline std::array<vk::VertexInputAttributeDescription, 3> GetVertexAttributeDescriptions()
{
    return {
        {{.location = 0, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, position)},
         {.location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, normal)},
         {.location = 2, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, texCoord)}}};
}
