cmake_minimum_required(VERSION 3.22)
project(scop-project LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(RENDERER_OPENGL "Enable OpenGL renderer" OFF)
option(RENDERER_METAL "Enable Metal renderer" OFF)
option(RENDERER_VULKAN "Enable Vulkan renderer" ON)

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

set(RENDERER_VULKAN_SOURCES
    src/core/renderer/vulkan/RendererVulkan.cpp
    src/core/renderer/vulkan/TextureVulkan.cpp
)


set(CORE_SOURCES
    src/camera.cpp
    src/Mesh.cpp
    src/core/Window.cpp
)

set(EXTERNAL_SOURCES
    third_party/glad/src/glad.c
)

add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/assets
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

if(RENDERER_VULKAN)
    target_compile_definitions(scop PRIVATE USE_VULKAN=1)
    target_sources(scop PRIVATE
        ${RENDERER_VULKAN_SOURCES}
    )
endif()

if(RENDERER_METAL)
    target_compile_definitions(scop PRIVATE USE_METAL=1)
endif()

if(RENDERER_VULKAN)
    find_package(Vulkan REQUIRED)
    target_compile_definitions(scop PRIVATE
        USE_VULKAN=1
        VULKAN_HPP_NO_CONSTRUCTORS
        VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
    )
    target_link_libraries(scop PRIVATE Vulkan::Vulkan)

    find_program(SLANGC_EXECUTABLE slangc
        HINTS "$ENV{VULKAN_SDK}/bin"
        REQUIRED
    )
    function(add_slang_shader_target TARGET)
        cmake_parse_arguments(PARSE_ARGV 1 SHADER "" "OUTPUT_NAME" "SOURCES;ENTRY_POINTS")
        if(NOT SHADER_SOURCES)
            message(FATAL_ERROR "${TARGET}: SOURCES is required")
        endif()
        if(NOT SHADER_OUTPUT_NAME)
            set(SHADER_OUTPUT_NAME "${TARGET}")
        endif()
        if(NOT SHADER_ENTRY_POINTS)
            set(SHADER_ENTRY_POINTS vertMain fragMain)
        endif()

        set(ENTRY_POINT_ARGS)
        foreach(ENTRY_POINT IN LISTS SHADER_ENTRY_POINTS)
            list(APPEND ENTRY_POINT_ARGS -entry "${ENTRY_POINT}")
        endforeach()

        set(SHADER_SOURCES_ABSOLUTE)
        foreach(SOURCE IN LISTS SHADER_SOURCES)
            get_filename_component(SOURCE_ABSOLUTE "${SOURCE}" ABSOLUTE
                BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
            list(APPEND SHADER_SOURCES_ABSOLUTE "${SOURCE_ABSOLUTE}")
        endforeach()

        set(SHADERS_DIR "${CMAKE_CURRENT_BINARY_DIR}/assets/shaders/spirv")
        set(SPIRV_OUTPUT "${SHADERS_DIR}/${SHADER_OUTPUT_NAME}.spv")
        add_custom_command(
            OUTPUT "${SPIRV_OUTPUT}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADERS_DIR}"
            COMMAND "${SLANGC_EXECUTABLE}" ${SHADER_SOURCES_ABSOLUTE}
                -target spirv
                -matrix-layout-column-major
                -profile spirv_1_4
                -emit-spirv-directly
                -fvk-use-entrypoint-name
                ${ENTRY_POINT_ARGS}
                -depfile "${SPIRV_OUTPUT}.d"
                -o "${SPIRV_OUTPUT}"
            DEPENDS ${SHADER_SOURCES_ABSOLUTE} "${SLANGC_EXECUTABLE}"
            DEPFILE "${SPIRV_OUTPUT}.d"
            COMMENT "Compiling ${SHADER_OUTPUT_NAME} to SPIR-V"
            VERBATIM
        )
        add_custom_target(${TARGET} DEPENDS "${SPIRV_OUTPUT}")
    endfunction()

    add_slang_shader_target(basic_shader
        SOURCES assets/shaders/slang/Basic.slang
        OUTPUT_NAME Basic
        ENTRY_POINTS vertMain fragMain
    )
    add_custom_target(compile_shaders)
    add_dependencies(compile_shaders basic_shader)
    add_dependencies(scop compile_shaders)
endif()

target_compile_definitions(scop PRIVATE
    ASSET_DIR="${CMAKE_BINARY_DIR}/assets"
)

add_dependencies(scop copy_assets)

include(FetchContent)
FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG 829a65d769d935c4852f8159e964312c0957260a
)
FetchContent_MakeAvailable(SDL3)

target_include_directories(scop PRIVATE
    third_party/glad/include
    include
)

target_link_libraries(scop PRIVATE SDL3::SDL3)
