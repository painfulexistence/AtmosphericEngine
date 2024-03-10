# Atmospheric Engine
**Atmospheric Engine** is a cross-platform 3D game engine developed in C++. As a labor of love, this project acts as my stepping stone to deepen my understanding of graphics programming concepts and practices. The engine's renderer currently supports HDR rendering, PBR materials, point and directional shadows, along with a handful of post-processing effects.
#### WARNING
This project is still under development and may be completely broken at this stage.

## Building
1. Install CMake
See https://cmake.org/install/
2. Install EMSDK (optional)
See https://emscripten.org/docs/getting_started/downloads.html
3. Clone the repo
```
git clone --recurse-submodules https://github.com/painfulexistence/AtmosphericEngine.git
cd AtmosphericEngine
```
4. Run CMake
```
mkdir build && cd build
cmake ..
```
#### Recommeded CMake options:
Turn off:
- BUILD_BULLET2_DEMOS
- BUILD_BULLET_ROBOTICS_EXTRA
- BUILD_BULLET_ROBOTICS_GUI_EXTRA
- BUILD_LUA_AS_DLL
- BUILD_UNIT_TESTS
- BUILD_SHARED_LIBS

## Source Structure
/AtmosphericEngine - Engine souce files
    /externel - Third party dependencies: stb, dearImgui, sol2, entt
/Example_MazeWorld - Example game source files 