cmake_minimum_required(VERSION 3.22)
project(scop-project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(COMMON_WARN_FLAGS "-Wall -Wextra -Werror")

set(CMAKE_CXX_FLAGS_DEBUG "${COMMON_WARN_FLAGS} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${COMMON_WARN_FLAGS} -O3")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type." FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if(APPLE)
    add_definitions(-DUSE_METAL)
    message(STATUS "Metal backend enabled for Apple platform")
else()
    add_definitions(-DUSE_OPENGL)
    message(STATUS "OpenGL backend enabled")
endif()

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

add_subdirectory(third_party/SDL)

target_include_directories(scop PRIVATE
    third_party/glad/include
    third_party/SDL/include
    include
)

target_link_libraries(scop PRIVATE SDL3::SDL3)