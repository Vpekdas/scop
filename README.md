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

Scop is a project from the 42 Post-Common Core curriculum, designed to introduce students to low-level graphics programming using APIs like Vulkan, Metal or OpenGL. 

### Purpose

The objective is to create a simple 3D OBJ renderer that displays textures and can rotate along all three axes.

## Controls

### Camera Movement
| Key        | Action |
|:----------:|--------|
| `W`        | Move camera forward |
| `S`        | Move camera backward |
| `A`        | Move camera left |
| `D`        | Move camera right |
| `←`        | Rotate camera left |
| `→`        | Rotate camera right |

### Model Controls
| Key   | Action |
|:-----:|--------|
| `X`   | Set rotation axis to X |
| `Y`   | Set rotation axis to Y |
| `Z`   | Set rotation axis to Z |
| `Space` | Stop model rotation |

---

### Rendering Modes
| Key | Action |
|:---:|--------|
| `F1` | Point mode |
| `F2` | Wireframe mode |
| `F3` | Fill mode |

---

### Effects
| Key | Action |
|:---:|--------|
| `F4` | Apply texture |
| `F5` | Apply dissolve effect |
| `F6` | Play Bad Apple |

---

### Audio
| Key | Action |
|:---:|--------|
| P   | Play / Pause audio |


### Global
| Key | Action |
|:---:|--------|
| ESC | Exit application |

### Technologies used

- C++
- CMake
- GLSL
- OpenGL
- SDL3

### Challenges and Future Features

The main challenge was understanding how the GPU renders pixels, and why triangles are important. Then I had to learn what OpenGL is and how to use it. Of course, I had to learn some basic mathematics, such as matrices, since matrix operations are not commutative. And lastly, how to use CMake.

I'm planning to add Metal and Vulkan backends later.

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
cmake -S . -B build
```

- Build the project using CMake:
```bash
cmake --build build --config Release
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

