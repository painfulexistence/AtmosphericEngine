# Atmospheric Engine
**Atmospheric Engine** is a cross-platform 3D game engine developed in C++. As a labor of love, this project acts as my stepping stone to deepen my understanding of graphics programming concepts and practices. The engine's renderer currently supports HDR rendering, PBR materials, point and directional shadows, along with a handful of post-processing effects.
#### WARNING
This project is still under development and may be completely broken at this stage.


## Building
1. Install [CMake](https://cmake.org/download/) (required) and [EMSDK](https://emscripten.org/docs/getting_started/downloads.html) (optional)
2. Clone this repo
```
git clone --recurse-submodules https://github.com/painfulexistence/AtmosphericEngine.git
cd AtmosphericEngine
./vcpkg/bootstrap-vcpkg.sh -disableMetrics
```
3. Run CMake to build the project
```
cmake -S . -B build
cmake --build build
```
#### CMake options:
WIP


## Source Structure
```
.
|- assets/                 # Static assets
|- AtmosphericEngine/      # Engine souce files
|  |- external/            # Third party dependencies, including stb, dearImgui, sol2, entt
|- Example_MazeWorld/      # Example game source files
|- vcpkg/                  # The package manager we're using
|- vcpkg.json              # Package metadata file
```


## Log
- 2024/03/22 - considering using glad+sdl2 instead of glew+glfw
- 2024/06/24 - upgraded dear imgui so that it can work on Apple M3 machines
- 2024/06/26 - fixed an issue where the error is not thrown when the framebuffer is incomplete (missing "throw" keyword)
- 2024/06/26 - fixed an issue where glGetString(GL_VERSION) returns null and all textures are missing and not loadable by switching to system-wide GLEW and back
- TODO: use std::filesystem to get the correct asset loading path