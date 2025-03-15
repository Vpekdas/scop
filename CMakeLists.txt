cmake_minimum_required(VERSION 3.22)

project(scop-project)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(scop
    src/IndexBuffer.cpp
    src/main.cpp
    src/Model.cpp
    src/Renderer.cpp
    src/Shader.cpp
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
