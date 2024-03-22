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
./vcpkg/bootstrap-vcpkg.sh
```
3. Run CMake to build the project
```
cmake -S . -B build
cmake --build build
```
#### Recommended CMake options:
Turn off:
- BUILD_BULLET2_DEMOS
- BUILD_BULLET_ROBOTICS_EXTRA
- BUILD_BULLET_ROBOTICS_GUI_EXTRA
- BUILD_LUA_AS_DLL
- BUILD_UNIT_TESTS
- BUILD_SHARED_LIBS


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
- TODO: use std::filesystem to get the correct asset loading path