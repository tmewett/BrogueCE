cmake_minimum_required(VERSION 3.22.1)
project(brogue C)

# ---- SDL2 built from source (submodules) ----

set(BROGUE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../..")
set(SDL2_SOURCE_DIR "${BROGUE_ROOT}/android/SDL2")
set(SDL2_IMAGE_SOURCE_DIR "${BROGUE_ROOT}/android/SDL2_image")

# Build SDL2 as a shared library
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
add_subdirectory(${SDL2_SOURCE_DIR} SDL2-build)

# SDL2_image needs to find our SDL2 build
set(SDL2IMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_VENDORED OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_PNG ON CACHE BOOL "" FORCE)
set(SDL2IMAGE_JPG OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_BMP OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_WEBP OFF CACHE BOOL "" FORCE)
add_subdirectory(${SDL2_IMAGE_SOURCE_DIR} SDL2_image-build)

# ---- BrogueCE source (unmodified) ----

file(GLOB BROGUE_SOURCES "${BROGUE_ROOT}/src/brogue/*.c")
file(GLOB VARIANT_SOURCES "${BROGUE_ROOT}/src/variants/*.c")

set(PLATFORM_SOURCES
    ${BROGUE_ROOT}/src/platform/main.c
    ${BROGUE_ROOT}/src/platform/platformdependent.c
    ${BROGUE_ROOT}/src/platform/null-platform.c
    ${BROGUE_ROOT}/src/platform/sdl2-platform.c
    ${BROGUE_ROOT}/src/platform/tiles.c
)

# ---- Android touch layer (the only new code) ----

set(ANDROID_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/android-touch.c
    ${CMAKE_CURRENT_SOURCE_DIR}/android-assets.c
)

# ---- Build shared library ----

add_library(brogue SHARED
    ${BROGUE_SOURCES}
    ${VARIANT_SOURCES}
    ${PLATFORM_SOURCES}
    ${ANDROID_SOURCES}
)

target_include_directories(brogue PRIVATE
    ${BROGUE_ROOT}/src/brogue
    ${BROGUE_ROOT}/src/platform
    ${BROGUE_ROOT}/src/variants
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(brogue PRIVATE
    DATADIR="."
    BROGUE_SDL
    SDL_PATHS
    BROGUE_ANDROID
    BROGUE_EXTRA_VERSION=""
)

# Force-include SDL_main.h so main() gets renamed to SDL_main() with proper
# export visibility. Without SDLMAIN_DECLSPEC the symbol isn't visible to JNI.
set_source_files_properties(
    ${BROGUE_ROOT}/src/platform/main.c
    PROPERTIES COMPILE_OPTIONS "-include;SDL_main.h"
)

target_link_libraries(brogue PRIVATE
    SDL2
    SDL2_image
    m
    log
    android
)
