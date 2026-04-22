cmake_minimum_required(VERSION 3.22)
project(scop-project LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(RENDERER_OPENGL "Enable OpenGL renderer" ON)
option(RENDERER_METAL "Enable Metal renderer" OFF)

set(APP_SOURCES
    src/app/main.cpp
    src/app/Application.cpp
)

set(RENDERER_OPENGL_SOURCES
    src/core/renderer/opengl/RendererOpenGL.cpp
    src/core/renderer/opengl/ShaderOpenGL.cpp
    src/core/renderer/opengl/TextureOpenGL.cpp
    src/core/renderer/opengl/VertexBuffer.cpp
    src/core/renderer/opengl/IndexBuffer.cpp
)

set(CORE_SOURCES
    src/camera.cpp
    src/Model.cpp
    src/math/vector.cpp
    src/core/Window.cpp
    src/core/AudioPlayer.cpp
)

set(EXTERNAL_SOURCES
    third_party/glad/src/glad.c
)


set(ASSET_ZIP ${CMAKE_SOURCE_DIR}/assets/textures/Bad_Apple.zip)
set(ASSET_DIR ${CMAKE_BINARY_DIR}/assets/)

add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/assets
)

add_custom_target(extract_assets ALL
    COMMAND ${CMAKE_COMMAND} -E tar -xf
        ${CMAKE_BINARY_DIR}/assets/textures/Bad_Apple.zip
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/assets/textures
    DEPENDS copy_assets
)

add_executable(scop
    ${APP_SOURCES}
    ${CORE_SOURCES}
    ${EXTERNAL_SOURCES}
)

target_compile_options(scop PRIVATE
    $<$<CONFIG:Debug>: -Wall -Wextra -Werror -g>
    $<$<CONFIG:Release>: -Wall -Wextra -Werror -O3>
)

if(RENDERER_OPENGL)
    target_compile_definitions(scop PRIVATE USE_OPENGL=1)
    target_sources(scop PRIVATE
        ${RENDERER_OPENGL_SOURCES}
    )
endif()

if(RENDERER_METAL)
    target_compile_definitions(scop PRIVATE USE_METAL=1)
endif()

target_compile_definitions(scop PRIVATE
    ASSET_DIR="${CMAKE_BINARY_DIR}/assets"
)

add_dependencies(scop extract_assets)

add_subdirectory(third_party/SDL)

target_include_directories(scop PRIVATE
    third_party/glad/include
    third_party/SDL/include
    include
)

target_link_libraries(scop PRIVATE SDL3::SDL3)