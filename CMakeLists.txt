cmake_minimum_required(VERSION 3.22)
project(scop-project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Remove global override of CXX_FLAGS, and instead set flags per configuration
# Define common warning flags
set(COMMON_WARN_FLAGS "-Wall -Wextra -Werror")

# Set flags based on build type
set(CMAKE_CXX_FLAGS_DEBUG "${COMMON_WARN_FLAGS} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${COMMON_WARN_FLAGS} -O3")

# Let user choose build type (default to Release if not specified)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type." FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Add executable and source files
add_executable(scop
    src/camera.cpp
    src/IndexBuffer.cpp
    src/main.cpp
    src/Model.cpp
    src/Renderer.cpp
    src/Shader.cpp
    src/Texture.cpp
    src/vector.cpp
    src/VertexBuffer.cpp
    third_party/glad/src/glad.c
)

# SDL
add_subdirectory(third_party/SDL)

# Include paths
target_include_directories(scop PRIVATE
    third_party/glad/include
    third_party/SDL/include
    include
)

# Link libraries
target_link_libraries(scop PRIVATE SDL3::SDL3)
