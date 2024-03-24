FROM mcr.microsoft.com/devcontainers/cpp:0-ubuntu-22.04

# Install CMake
RUN if ! [command -v cmake &> /dev/null]; then \
        apt-get install cmake; \
    fi \
    && echo "CMake version: $(which cmake)"

# Set Emscripten variables
ENV EMSDK="usr/local/emsdk" 

# Set Vcpkg variables to workaround https://github.com/msclock/features/issues/15
ENV VCPKG_FORCE_SYSTEM_BINARIES=1

# Install dependencies
RUN apt-get update && apt-get install -y libxmu-dev libxi-dev libgl-dev libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config

# Build
# WORKDIR /workspaces/AtmosphericEngine
# RUN cmake -S . -B build-linux -DUSE_GLOBAL_VCPKG=true -DCMAKE_MAKE_PROGRAM="/usr/bin/make" -DCMAKE_C_COMPILER="/usr/bin/gcc" -DCMAKE_CXX_COMPILER="/usr/bin/g++"
# RUN cmake --build build-linux