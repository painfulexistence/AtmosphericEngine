set(VCPKG_TARGET_ARCHITECTURE wasm32)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Emscripten)

# Point vcpkg at the Emscripten CMake toolchain (same as the community triplet).
# setup-emsdk exports $EMSDK; fall back to $EMSCRIPTEN if available.
if(DEFINED ENV{EMSDK})
    set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE
        "$ENV{EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")
elseif(DEFINED ENV{EMSCRIPTEN})
    set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE
        "$ENV{EMSCRIPTEN}/cmake/Modules/Platform/Emscripten.cmake")
endif()

# Pass -matomics -mbulk-memory directly so every port's object files support
# WASM shared memory.  -pthread alone may be silently dropped by Emscripten's
# CMake integration when building static libraries via vcpkg.
set(VCPKG_C_FLAGS "-matomics -mbulk-memory")
set(VCPKG_CXX_FLAGS "-matomics -mbulk-memory")
