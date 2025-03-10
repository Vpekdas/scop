cmake_minimum_required(VERSION 3.22)

project(scop-project)

add_executable(scop
    src/main.cpp
    third_party/glad/src/glad.c
)

add_subdirectory(third_party/SDL)

target_include_directories(scop PRIVATE
    third_party/glad/include
    third_party/SDL/include
)

target_link_libraries(scop PRIVATE SDL3::SDL3)
