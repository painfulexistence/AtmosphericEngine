# What is Atmospheric?
**Atmospheric** is an open-source, cross-platform 3D game engine built in C++. This is a project I do in my free time for learning graphics programming concepts & practices. The renderer currently supports HDR rendering, PBR materials, point shadows, directional shadows, and a handful of post-processing effects.

# WARNING
This project is still under development and may be completely broken at this stage.

# Building
## Install CMake
## Install VCPKG
## Install EMSDK (optional)
See https://emscripten.org/docs/getting_started/downloads.html
## Run CMake
```
mkdir build
cd build && cmake ..
```

#### Recommeded CMake options:
Turn off:
- BUILD_BULLET2_DEMOS
- BUILD_BULLET_ROBOTICS_EXTRA
- BUILD_BULLET_ROBOTICS_GUI_EXTRA
- BUILD_LUA_AS_DLL
- BUILD_UNIT_TESTS
- BUILD_SHARED_LIBS

# Project structure
/src
    source files
/external
    dependencies: bullet3, entt, fmt, glfw, sol2, dearImgui, stb