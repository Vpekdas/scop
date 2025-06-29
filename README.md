# Scop

## Screenshots


https://github.com/user-attachments/assets/f34b98ba-7047-4418-b626-537d97f572d0


## Table of Contents
1. [Description](#description)
2. [Installation](#installation)
3. [Run](#run)
4. [Credits](#credits)
5. [Contributing](#contributing)
6. [License](#license)

## Description

Scop is a project from the 42 Post-Common Core curriculum, designed to introduce students to low-level graphics programming using APIs like Vulkan or OpenGL. 

### Purpose

The objective is to create a simple 3D OBJ renderer that displays textures and can rotate along all three axes.

## Controls

| **Key** | **Action** |
|:-------:|------------|
| `X`     | Rotate model around X-axis |
| `Y`     | Rotate model around Y-axis |
| `Z`     | Rotate model around Z-axis |
| `Space` | Pause rotation |
| `→`     | Rotate right |
| `←`     | Rotate left |
| `T`     | Enable texture mode |
| `F`     | Enable face mode |
| `F1`    | Switch to point mode |
| `F2`    | Switch to line mode |
| `F3`    | Switch to fill mode |
| `ESC` or click the `X` (top-right corner) | Exit |

### Technologies used

- C++: The primary programming language for the project.
- CMake: A tool for managing the build process.
- GLSL: The shading language used for writing shaders.
- OpenGL: The graphics API for rendering the 3D model and textures.

### Challenges and Future Features

The main challenges in this project were learning how graphics APIs, like OpenGL, work. Initially, I struggled to understand why triangles are the preferred primitive for rendering. The difficulty increased when I had to render the triangle by creating an index buffer, vertex arrays, and setting up vertex attribute pointers. After that, I moved on to creating a cube and implementing rotations along all three axes.

Before tackling these challenges, I also had to learn some basic mathematics, such as understanding matrices, how they work, and how they are used in 3D graphics.

I'm not planning to add new features.

## Installation

- Ensure you have a C compiler installed, such as [Clang](https://clang.llvm.org/) or [GCC](https://gcc.gnu.org/)
```bash
clang --version
gcc --version
```
- Ensure you have installed [CMake](https://cmake.org/) to build the project
```bash
cmake --version
```

- Create a build folder to organize the build files:
```bash
mkdir build
```

- Generate build files with CMake:
```bash
cmake -S . -B build/Release -DCMAKE_BUILD_TYPE=RELEASE
```

- Build the project using CMake:
```bash
cmake --build build/Release --config Release
```

> [!NOTE]
On macOS, OpenGL is deprecated, so you might encounter warnings during the build process. However, these warnings do not cause any issues and can be safely ignored.

## Run

- To run the program, provide the path to an OBJ model and its corresponding TGA texture file as arguments:

```bash
./build/Release/scop [./assets/models/.obj] [./assets/textures/.tga]
```

## Credits


- [FirePh0enix](https://github.com/FirePh0enix): Thanks for your help in choosing an API and advising that OpenGL would be easier to learn. Also, for explaining the basics of matrices and fan triangulation

- [The Cherno](https://www.youtube.com/@TheCherno): Thanks for his great videos on OpenGL and, overall, for sharing his knowledge on C++ and game engine development!

## Contributing

To report issues, please create an issue here:  [issue tracker](https://github.com/Vpekdas/scop/issues).

If you'd like to contribute, please follow the steps outlined in [CONTRIBUTING.md](CONTRIBUTING.md).

## License

This project is licensed under the [MIT License](LICENSE).

### Third-Party Licenses

- **SDL** - Licensed under the [zlib License](third_party/SDL/README.md).
- **Glad** - Licensed under [MIT License](third_party/glad/include/KHR/khrplatform.h).

