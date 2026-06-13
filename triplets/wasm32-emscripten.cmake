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

# Enable WASM atomics so libraries compiled with the multithreading feature
# (e.g. bullet3) produce object files compatible with --shared-memory linking.
# Required when the consumer uses -sUSE_PTHREADS=1 / -sPROXY_TO_PTHREAD=1.
set(VCPKG_C_FLAGS "-pthread -matomics -mbulk-memory")
set(VCPKG_CXX_FLAGS "-pthread -matomics -mbulk-memory")
