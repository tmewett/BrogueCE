cmake_minimum_required(VERSION 3.16)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# See docs/release_checklist.md
set(MAJOR_VERSION 2)
set(MINOR_VERSION 8)
set(MICRO_VERSION 9)
set(SDL_REQUIRED_VERSION 2.0.9)

include(PrivateSdlFunctions)
sdl_calculate_derived_version_variables()

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the SDL_image source code and call cmake from there")
endif()

project(SDL2_image
    LANGUAGES C
    VERSION "${FULL_VERSION}"
)

message(STATUS "Configuring ${PROJECT_NAME} ${PROJECT_VERSION}")

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(SDL2IMAGE_ROOTPROJECT ON)
else()
    set(SDL2IMAGE_ROOTPROJECT OFF)
endif()
set(SDL2IMAGE_SAMPLES_DEFAULT ${SDL2IMAGE_ROOTPROJECT})
if(ANDROID)
    set(SDL2IMAGE_SAMPLES_DEFAULT OFF)
endif()

# Set defaults preventing destination file conflicts
set(SDL2IMAGE_DEBUG_POSTFIX "d"
    CACHE STRING "Name suffix for debug builds")
mark_as_advanced(SDL2IMAGE_DEBUG_POSTFIX)

# Assume MSVC projects don't have a package manager and need vendored dependencies (by default).
# Most other platforms have some kind of package manager.
# FIXME: consider a package manager such as conan/vcpkg instead of vendoring
if(MSVC)
    set(vendored_default ON)
else()
    set(vendored_default OFF)
endif()

set(sdl2image_install_enableable ON)
if ((TARGET SDL2 OR TARGET SDL2-static) AND SDL2_DISABLE_INSTALL)
    # Cannot install SDL2_image when SDL2 is built in same built, and is not installed.
    set(sdl2image_install_enableable OFF)
endif()

include(CMakeDependentOption)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

option(CMAKE_POSITION_INDEPENDENT_CODE "Build static libraries with -fPIC" ON)
option(BUILD_SHARED_LIBS "Build the library as a shared library" ON)

cmake_dependent_option(SDL2IMAGE_INSTALL "Enable SDL2_image install target" ${SDL2IMAGE_ROOTPROJECT} "${sdl2image_install_enableable}" OFF)
option(SDL2IMAGE_DEPS_SHARED "Load dependencies dynamically" ON)
option(SDL2IMAGE_VENDORED "Use vendored third-party libraries" ${vendored_default})

option(SDL2IMAGE_STRICT "Fail when a dependency could not be found" OFF)
set(required "")
set(fatal_error "STATUS")
if(SDL2IMAGE_STRICT)
    set(required "REQUIRED")
    set(fatal_error "FATAL_ERROR")
endif()

option(SDL2IMAGE_SAMPLES "Build the SDL2_image sample program(s)" ${SDL2IMAGE_SAMPLES_DEFAULT})
cmake_dependent_option(SDL2IMAGE_SAMPLES_INSTALL "Install the SDL2_image sample program(s)" OFF "SDL2IMAGE_SAMPLES;SDL2IMAGE_INSTALL" OFF)

option(SDL2IMAGE_TESTS "Build unit tests?" OFF)
cmake_dependent_option(SDL2IMAGE_TESTS_INSTALL "Install unit tests?" OFF "SDL2IMAGE_TESTS;SDL2IMAGE_INSTALL" OFF)

option(SDL2IMAGE_BACKEND_STB "Use stb_image for loading JPEG and PNG files" ON)
cmake_dependent_option(SDL2IMAGE_BACKEND_WIC "Add WIC backend (Windows Imaging Component)" OFF WIN32 OFF)
cmake_dependent_option(SDL2IMAGE_BACKEND_IMAGEIO "Use native Mac OS X frameworks for loading images" ON APPLE OFF)

option(SDL2IMAGE_AVIF "Support loading AVIF images" ON)
option(SDL2IMAGE_BMP "Support loading BMP images" ON)
option(SDL2IMAGE_GIF "Support loading GIF images" ON)
option(SDL2IMAGE_JPG "Support loading JPEG images" ON)
option(SDL2IMAGE_JXL "Support loading JXL images" OFF)
option(SDL2IMAGE_LBM "Support loading LBM images" ON)
option(SDL2IMAGE_PCX "Support loading PCX images" ON)
option(SDL2IMAGE_PNG "Support loading PNG images" ON)
option(SDL2IMAGE_PNM "Support loading PNM images" ON)
option(SDL2IMAGE_QOI "Support loading QOI images" ON)
option(SDL2IMAGE_SVG "Support loading SVG images" ON)
option(SDL2IMAGE_TGA "Support loading TGA images" ON)
option(SDL2IMAGE_TIF "Support loading TIFF images" ON)
option(SDL2IMAGE_WEBP "Support loading WEBP images" ON)
option(SDL2IMAGE_XCF "Support loading XCF images" ON)
option(SDL2IMAGE_XPM "Support loading XPM images" ON)
option(SDL2IMAGE_XV "Support loading XV images" ON)

cmake_dependent_option(SDL2IMAGE_JPG_SAVE "Add JPEG save support" ON SDL2IMAGE_JPG OFF)
cmake_dependent_option(SDL2IMAGE_PNG_SAVE "Add PNG save support" ON SDL2IMAGE_PNG OFF)

set(LIBAVIF_MINIMUM_VERSION "0.9.3")
if(SDL2IMAGE_VENDORED AND SDL2IMAGE_AVIF)
    set(SDL2IMAGE_AVIF_VENDORED ON)
else()
    set(SDL2IMAGE_AVIF_VENDORED OFF)
endif()
cmake_dependent_option(SDL2IMAGE_AVIF_SHARED "Dynamically load AVIF support (requires shared libavif)"
    ${SDL2IMAGE_DEPS_SHARED} SDL2IMAGE_AVIF OFF)

if(SDL2IMAGE_AVIF_VENDORED)
  set(SDL2IMAGE_DAV1D ON)
  set(SDL2IMAGE_DAV1D_VENDORED ON)
else()
  set(SDL2IMAGE_DAV1D OFF)
  set(SDL2IMAGE_DAV1D_VENDORED OFF)
endif()
if(SDL2IMAGE_AVIF_SHARED)
  set(SDL2IMAGE_DAV1D_SHARED ON)
else()
  set(SDL2IMAGE_DAV1D_SHARED OFF)
endif()

if(SDL2IMAGE_VENDORED AND SDL2IMAGE_JPG AND NOT (SDL2IMAGE_BACKEND_WIC OR SDL2IMAGE_BACKEND_STB OR SDL2IMAGE_BACKEND_IMAGEIO))
    set(SDL2IMAGE_JPG_VENDORED ON)
else()
    set(SDL2IMAGE_JPG_VENDORED OFF)
endif()
cmake_dependent_option(SDL2IMAGE_JPG_SHARED "Dynamically load JPG support (requires shared libjpeg)"
    ${SDL2IMAGE_DEPS_SHARED} "SDL2IMAGE_JPG;NOT SDL2IMAGE_BACKEND_WIC;NOT SDL2IMAGE_BACKEND_STB;NOT SDL2IMAGE_BACKEND_IMAGEIO" OFF)

if(SDL2IMAGE_VENDORED AND SDL2IMAGE_JXL)
    set(SDL2IMAGE_JXL_VENDORED ON)
else()
    set(SDL2IMAGE_JXL_VENDORED OFF)
endif()
cmake_dependent_option(SDL2IMAGE_JXL_SHARED "Dynamically load JXL support (requires shared libjxl)"
    ${SDL2IMAGE_DEPS_SHARED} SDL2IMAGE_JXL OFF)

if(SDL2IMAGE_VENDORED AND SDL2IMAGE_PNG AND NOT (SDL2IMAGE_BACKEND_WIC OR SDL2IMAGE_BACKEND_STB OR SDL2IMAGE_BACKEND_IMAGEIO))
    set(SDL2IMAGE_PNG_VENDORED ON)
else()
    set(SDL2IMAGE_PNG_VENDORED OFF)
endif()
cmake_dependent_option(SDL2IMAGE_PNG_SHARED "Dynamically load PNG support (requires shared libpng)"
    ${SDL2IMAGE_DEPS_SHARED} "SDL2IMAGE_PNG;NOT SDL2IMAGE_BACKEND_WIC;NOT SDL2IMAGE_BACKEND_STB;NOT SDL2IMAGE_BACKEND_IMAGEIO" OFF)

if(SDL2IMAGE_VENDORED AND SDL2IMAGE_TIF)
    set(SDL2IMAGE_TIF_VENDORED ON)
else()
    set(SDL2IMAGE_TIF_VENDORED OFF)
endif()
cmake_dependent_option(SDL2IMAGE_TIF_SHARED "Dynamically load TIFF support (requires shared libtiff)"
    ${SDL2IMAGE_DEPS_SHARED} SDL2IMAGE_TIF OFF)

if(SDL2IMAGE_VENDORED AND SDL2IMAGE_WEBP)
    set(SDL2IMAGE_WEBP_VENDORED ON)
else()
    set(SDL2IMAGE_WEBP_VENDORED OFF)
endif()
cmake_dependent_option(SDL2IMAGE_WEBP_SHARED "Dynamically load WEBP support (requires shared libwebp)"
    ${SDL2IMAGE_DEPS_SHARED} SDL2IMAGE_WEBP OFF)

if(SDL2IMAGE_PNG_VENDORED)
    set(SDL2IMAGE_ZLIB ON)
else()
    set(SDL2IMAGE_ZLIB OFF)
endif()

if(SDL2IMAGE_VENDORED AND SDL2IMAGE_PNG_VENDORED)
    set(SDL2IMAGE_ZLIB_VENDORED ON)
else()
    set(SDL2IMAGE_ZLIB_VENDORED OFF)
endif()
set(SDL2IMAGE_ZLIB_SHARED OFF)

# Save BUILD_SHARED_LIBS variable
set(SDL2IMAGE_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

if(SDL2IMAGE_BUILD_SHARED_LIBS)
    set(sdl2_image_export_name SDL2_image)
    set(sdl2_image_install_name_infix shared)
    set(sdl2_target_name SDL2::SDL2)
else()
    set(sdl2_image_export_name SDL2_image-static)
    set(sdl2_image_install_name_infix static)
    set(sdl2_target_name SDL2::SDL2-static)
endif()

sdl_find_sdl2(${sdl2_target_name} ${SDL_REQUIRED_VERSION})

# Set PROJECT_VERSION of subprojects to "" if it's project call does not set VERSION
cmake_policy(SET CMP0048 NEW)

# Allow cmake_dependent_option to use "Full Condition Syntax"
if(POLICY CMP0127)
    cmake_policy(SET CMP0127 NEW)
endif()

# OpenGL is required by dependencies of (dependencies of) some vendored libraries
if(NOT DEFINED OpenGL_GL_PREFERENCE)
    set(OpenGL_GL_PREFERENCE GLVND)
endif()

add_library(SDL2_image
    src/IMG.c
    src/IMG_WIC.c
    src/IMG_avif.c
    src/IMG_bmp.c
    src/IMG_gif.c
    src/IMG_jpg.c
    src/IMG_jxl.c
    src/IMG_lbm.c
    src/IMG_pcx.c
    src/IMG_png.c
    src/IMG_pnm.c
    src/IMG_qoi.c
    src/IMG_stb.c
    src/IMG_svg.c
    src/IMG_tga.c
    src/IMG_tif.c
    src/IMG_webp.c
    src/IMG_xcf.c
    src/IMG_xpm.c
    src/IMG_xv.c
)
add_library(SDL2_image::${sdl2_image_export_name} ALIAS SDL2_image)
target_include_directories(SDL2_image PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/SDL2>"
)
target_compile_definitions(SDL2_image PRIVATE
    BUILD_SDL
    SDL_BUILD_MAJOR_VERSION=${MAJOR_VERSION}
    SDL_BUILD_MINOR_VERSION=${MINOR_VERSION}
    SDL_BUILD_MICRO_VERSION=${MICRO_VERSION}
)
target_link_libraries(SDL2_image PRIVATE $<BUILD_INTERFACE:${sdl2_target_name}>)
if(WIN32 AND SDL2IMAGE_BUILD_SHARED_LIBS)
    target_sources(SDL2_image PRIVATE
        src/version.rc
    )
    if(MINGW)
        target_link_options(SDL2_image PRIVATE -static-libgcc)
    endif()
endif()
set_target_properties(SDL2_image PROPERTIES
    DEFINE_SYMBOL DLL_EXPORT
    EXPORT_NAME ${sdl2_image_export_name}
    C_VISIBILITY_PRESET "hidden"
)
if(NOT ANDROID)
    set_target_properties(SDL2_image PROPERTIES
        DEBUG_POSTFIX "${SDL2IMAGE_DEBUG_POSTFIX}"
        SOVERSION "${LT_MAJOR}"
        VERSION "${LT_VERSION}"
    )
    if(APPLE)
        cmake_minimum_required(VERSION 3.17)
        set_target_properties(SDL2_image PROPERTIES
            MACHO_COMPATIBILITY_VERSION "${DYLIB_COMPATIBILITY_VERSION}"
            MACHO_CURRENT_VERSION "${DYLIB_CURRENT_VERSION}"
        )
    endif()
endif()
if(SDL2IMAGE_BUILD_SHARED_LIBS AND (APPLE OR (UNIX AND NOT ANDROID)))
    add_custom_command(TARGET SDL2_image POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E create_symlink "$<TARGET_SONAME_FILE_NAME:SDL2_image>" "libSDL2_image$<$<CONFIG:Debug>:${SDL2IMAGE_DEBUG_POSTFIX}>$<TARGET_FILE_SUFFIX:SDL2_image>"
        # BYPRODUCTS "libSDL2_image$<$<CONFIG:Debug>:${SDL2IMAGE_DEBUG_POSTFIX}>$<TARGET_FILE_SUFFIX:SDL2_image>" # Needs CMake 3.20
        WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
    )
endif()
if(SDL2IMAGE_BUILD_SHARED_LIBS)
    if(WIN32 OR OS2)
        set_target_properties(SDL2_image PROPERTIES
            PREFIX ""
        )
    endif()
    if(OS2)
        # OS/2 doesn't support a DLL name longer than 8 characters.
        set_target_properties(SDL2_image PROPERTIES
            OUTPUT_NAME "SDL2img"
        )
    elseif(UNIX AND NOT ANDROID)
        set_target_properties(SDL2_image PROPERTIES
            OUTPUT_NAME "SDL2_image-${LT_RELEASE}"
        )
    endif()
else()
    if(MSVC OR (WATCOM AND (WIN32 OR OS2)))
        set_target_properties(SDL2_image PROPERTIES
            OUTPUT_NAME "SDL2_image-static"
        )
    endif()
endif()

# Use `Compatible Interface Properties` to ensure a shared SDL2_image is built with a shared SDL2
if(SDL2IMAGE_BUILD_SHARED_LIBS)
    set_property(TARGET SDL2_image PROPERTY INTERFACE_SDL2_SHARED ${SDL2IMAGE_BUILD_SHARED_LIBS})
    set_property(TARGET SDL2_image APPEND PROPERTY COMPATIBLE_INTERFACE_BOOL SDL2_SHARED)
endif()

if(SDL2IMAGE_BUILD_SHARED_LIBS)
    sdl_target_link_options_no_undefined(SDL2_image)
endif()

if(SDL2IMAGE_BUILD_SHARED_LIBS)
    # Make sure static library dependencies are built with -fPIC when building a shared SDL2_image
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

set(INSTALL_EXTRA_TARGETS)
set(PC_LIBS)
set(PC_REQUIRES)
set(SDL2IMAGE_BACKENDS)

list(APPEND SDL2IMAGE_BACKENDS STB)
set(SDL2IMAGE_STB_ENABLED FALSE)
if(SDL2IMAGE_BACKEND_STB)
    set(SDL2IMAGE_STB_ENABLED TRUE)
    target_compile_definitions(SDL2_image PRIVATE USE_STBIMAGE)
endif()

list(APPEND SDL2IMAGE_BACKENDS IMAGEIO)
set(SDL2IMAGE_IMAGEIO_ENABLED FALSE)
if(APPLE)
    if(SDL2IMAGE_BACKEND_IMAGEIO)
        set(SDL2IMAGE_IMAGEIO_ENABLED TRUE)
        if(CMAKE_SYSTEM_NAME MATCHES ".*(Darwin|MacOS).*")
            target_link_libraries(SDL2_image PRIVATE -Wl,-framework,ApplicationServices)
        endif()
        target_link_libraries(SDL2_image PRIVATE objc)
        target_sources(SDL2_image PRIVATE
            src/IMG_ImageIO.m
        )

        if (SDL2IMAGE_PNG AND NOT SDL2IMAGE_BACKEND_STB)
            target_compile_definitions(SDL2_image PRIVATE PNG_USES_IMAGEIO)
        endif()
        if (SDL2IMAGE_JPG AND NOT SDL2IMAGE_BACKEND_STB)
            target_compile_definitions(SDL2_image PRIVATE JPG_USES_IMAGEIO)
        endif()
    else()
        target_compile_definitions(SDL2_image PRIVATE SDL_IMAGE_USE_COMMON_BACKEND)
    endif()
endif()

list(APPEND SDL2IMAGE_BACKENDS WIC)
set(SDL2IMAGE_WIC_ENABLED FALSE)
if(SDL2IMAGE_BACKEND_WIC)
    set(SDL2IMAGE_WIC_ENABLED TRUE)
    target_compile_definitions(SDL2_image PRIVATE SDL_IMAGE_USE_WIC_BACKEND)
endif()

if(SDL2IMAGE_ZLIB)
    if(SDL2IMAGE_ZLIB_VENDORED)
        message(STATUS "${PROJECT_NAME}: Using vendored zlib")
        # disable build of zlib example programs:
        set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "zlib examples" FORCE)
        sdl_check_project_in_subfolder(external/zlib zlib SDL2IMAGE_VENDORED)
        add_subdirectory(external/zlib EXCLUDE_FROM_ALL)
        set(ZLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/external/zlib;${CMAKE_CURRENT_BINARY_DIR}/external/zlib")
        # ZLIB_INCLUDE_DIR variable is used by vendored libpng
        set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/zlib;${CMAKE_CURRENT_BINARY_DIR}/external/zlib" CACHE STRING "path of zlib, passed to libpng" FORCE)
        # ZLIB_LIBRARY variable is used by vendored libpng
        if(SDL2IMAGE_ZLIB_SHARED)
            set(ZLIB_LIBRARY zlib)
        else()
            set(ZLIB_LIBRARY zlibstatic)
            set(SKIP_INSTALL_CONFIG_FILE ON)
        endif()
        if(NOT TARGET ZLIB::ZLIB)
            add_library(ZLIB::ZLIB ALIAS ${ZLIB_LIBRARY})
        endif()
        if(NOT TARGET ZLIB::zlibstatic)
            add_library(ZLIB::zlibstatic ALIAS ${ZLIB_LIBRARY})
        endif()
        # ZLIB_INCLUDE_DIRS variable is used by vendored libpng
        set(ZLIB_INCLUDE_DIRS "${ZLIB_INCLUDE_DIR}")
        # ZLIB_LIBRARIES variable is used by vendored libpng
        set(ZLIB_LIBRARIES "${ZLIB_LIBRARY}")
        list(APPEND INSTALL_EXTRA_TARGETS ${ZLIB_LIBRARY})
        set_target_properties(${ZLIB_LIBRARY} PROPERTIES EXPORT_NAME external_zlib)
        add_library(SDL2_image::external_zlib ALIAS ${ZLIB_LIBRARY})
    else()
        message(FATAL_ERROR "Internal error (zlib is only required when requesting vendored dependencies)")
    endif()
endif()

if(SDL2IMAGE_DAV1D)
    if(SDL2IMAGE_DAV1D_VENDORED)
        message(STATUS "${PROJECT_NAME}: Using vendored dav1d")
        add_subdirectory(external/dav1d)
        set(DAV1D_LIBRARY dav1d)
        list(APPEND INSTALL_EXTRA_TARGETS dav1d)
    else()
        message(FATAL_ERROR "Internal error (dav1d is only required when requesting vendored dependencies)")
    endif()
endif()

list(APPEND SDL2IMAGE_BACKENDS AVIF)
set(SDL2IMAGE_AVIF_ENABLED FALSE)
if(SDL2IMAGE_AVIF)
    if(SDL2IMAGE_AVIF_VENDORED)
        set(SDL2IMAGE_AVIF_ENABLED TRUE)
        message(STATUS "${PROJECT_NAME}: Using vendored libavif")
        sdl_check_project_in_subfolder(external/libavif libavif SDL2IMAGE_VENDORED)
        set(BUILD_SHARED_LIBS ${SDL2IMAGE_AVIF_SHARED})
        set(AVIF_CODEC_DAV1D ON CACHE BOOL "Use dav1d codec for decoding" FORCE)
        set(AVIF_LOCAL_DAV1D OFF CACHE BOOL "Build dav1d by libaf" FORCE)
        #FIXME: This requires upgrading vendored libwebp to >= 1.2.4.
        #set(LIBAVIF_WITH_SHARPYUV_SDLIMAGE ${SDL2IMAGE_WEBP} CACHE BOOL "Build libavif with sharpyuv support (re-use sharpyuv built by libwebp)" FORCE)
        add_subdirectory(external/libavif EXCLUDE_FROM_ALL)
        list(APPEND INSTALL_EXTRA_TARGETS avif)
        set_target_properties(avif PROPERTIES EXPORT_NAME external_libavif)
        add_library(SDL2_image::external_libavif ALIAS avif)
        if(NOT SDL2IMAGE_AVIF_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:avif>)
        endif()
        if(NOT MSVC AND NOT CMAKE_SYSTEM_NAME MATCHES ".*OpenBSD.*")
          check_linker_flag(C "-Wl,--no-undefined" LINKER_SUPPORTS_WL_NO_UNDEFINED)
          if(LINKER_SUPPORTS_WL_NO_UNDEFINED)
            target_link_options(avif PRIVATE "-Wl,--no-undefined")
          endif()
        endif()
    else()
        find_package(libavif 1.0 QUIET)
        if(NOT libavif_FOUND)
            message(STATUS "libavif-1.0 or compatible not found")
            find_package(libavif ${LIBAVIF_MINIMUM_VERSION} QUIET)
        endif()
        if(libavif_FOUND)
            set(SDL2IMAGE_AVIF_ENABLED TRUE)
            message(STATUS "${PROJECT_NAME}: Using system libavif")
            message(STATUS "libavif-${libavif_VERSION} found")
            if(NOT SDL2IMAGE_AVIF_SHARED)
                list(APPEND PC_REQUIRES libavif)
            endif()
        else()
            message(STATUS "libavif-${LIBAVIF_MINIMUM_VERSION} or compatible not found")
            message(${fatal_error} "libavif NOT found")
        endif()
    endif()
    if(SDL2IMAGE_AVIF_ENABLED)
        target_compile_definitions(SDL2_image PRIVATE LOAD_AVIF)
        if(SDL2IMAGE_AVIF_SHARED)
            target_include_directories(SDL2_image PRIVATE
                $<TARGET_PROPERTY:avif,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:avif,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:avif,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_avif avif)
            message(STATUS "Dynamic libavif: ${dynamic_avif}")
            target_compile_definitions(SDL2_image PRIVATE "LOAD_AVIF_DYNAMIC=\"${dynamic_avif}\"")
            if(SDL2IMAGE_AVIF_VENDORED)
                add_dependencies(SDL2_image avif)
            endif()
        else()
            target_link_libraries(SDL2_image PRIVATE avif)
        endif()
    endif()
endif()

list(APPEND SDL2IMAGE_BACKENDS BMP)
set(SDL2IMAGE_BMP_ENABLED FALSE)
if(SDL2IMAGE_BMP)
    set(SDL2IMAGE_BMP_ENABLED TRUE)
    target_compile_definitions(SDL2_image PRIVATE LOAD_BMP)
endif()

list(APPEND SDL2IMAGE_BACKENDS GIF)
set(SDL2IMAGE_GIF_ENABLED FALSE)
if(SDL2IMAGE_GIF)
    set(SDL2IMAGE_GIF_ENABLED TRUE)
    target_compile_definitions(SDL2_image PRIVATE LOAD_GIF)
endif()

list(APPEND SDL2IMAGE_BACKENDS JPG)
set(SDL2IMAGE_JPG_ENABLED FALSE)
if(SDL2IMAGE_JPG)
    if(SDL2IMAGE_BACKEND_STB OR SDL2IMAGE_BACKEND_WIC OR SDL2IMAGE_BACKEND_IMAGEIO)
        set(SDL2IMAGE_JPG_ENABLED TRUE)
    else()
        if(SDL2IMAGE_JPG_VENDORED)
            set(SDL2IMAGE_JPG_ENABLED TRUE)
            message(STATUS "${PROJECT_NAME}: Using vendored libjpeg")
            sdl_check_project_in_subfolder(external/jpeg libjpeg SDL2IMAGE_VENDORED)
            set(BUILD_SHARED_LIBS ${SDL2IMAGE_JPG_SHARED})
            add_subdirectory(external/jpeg EXCLUDE_FROM_ALL)
            list(APPEND INSTALL_EXTRA_TARGETS jpeg)
            set_target_properties(jpeg PROPERTIES EXPORT_NAME external_libjpeg)
            add_library(SDL2_image::external_libjpeg ALIAS jpeg)
            if(NOT SDL2IMAGE_JPG_SHARED)
                list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:jpeg>)
            endif()
        else()
            find_package(JPEG ${required})
            if(JPEG_FOUND)
                set(SDL2IMAGE_JPG_ENABLED TRUE)
                message(STATUS "${PROJECT_NAME}: Using system libjpeg")
                if(NOT SDL2IMAGE_JPG_SHARED)
                    list(APPEND PC_REQUIRES libjpeg)
                endif()
            else()
                message(${fatal_error} "libjpeg NOT found")
            endif()
        endif()
        if(SDL2IMAGE_JPG_ENABLED)
            if(SDL2IMAGE_JPG_SHARED)
                target_include_directories(SDL2_image PRIVATE
                    $<TARGET_PROPERTY:JPEG::JPEG,INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:JPEG::JPEG,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:JPEG::JPEG,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
                )
                target_get_dynamic_library(dynamic_jpeg JPEG::JPEG)
                message(STATUS "Dynamic libjpeg: ${dynamic_jpeg}")
                target_compile_definitions(SDL2_image PRIVATE "LOAD_JPG_DYNAMIC=\"${dynamic_jpeg}\"")
                if(SDL2IMAGE_JPG_VENDORED)
                    add_dependencies(SDL2_image JPEG::JPEG)
                endif()
            else()
                target_link_libraries(SDL2_image PRIVATE JPEG::JPEG)
            endif()
        endif()
    endif()
    if(SDL2IMAGE_JPG_ENABLED)
        target_compile_definitions(SDL2_image PRIVATE
            LOAD_JPG
            SDL_IMAGE_SAVE_JPG=$<BOOL:${SDL2IMAGE_JPG_SAVE}>
        )
    endif()
endif()

list(APPEND SDL2IMAGE_BACKENDS JXL)
set(SDL2IMAGE_JXL_ENABLED FALSE)
if(SDL2IMAGE_JXL)
    if(SDL2IMAGE_JXL_VENDORED)
        set(SDL2IMAGE_JXL_ENABLED TRUE)
        enable_language(CXX)
        message(STATUS "${PROJECT_NAME}: Using vendored libjxl")
        # BUILD_TESTING variable is used by libjxl
        set(BUILD_TESTING OFF CACHE BOOL "build testing" FORCE)
        # JPEGXL_ENABLE_TOOLS variable is used by libjxl
        set(JPEGXL_ENABLE_JNI OFF CACHE BOOL "build jpegxl jni" FORCE)
        # JPEGXL_ENABLE_SJPEG variable is used by libjxl
        set(JPEGXL_ENABLE_SJPEG OFF CACHE BOOL "build jpegxl sjpeg" FORCE)
        # JPEGXL_BUNDLE_SKCMS variable is used by libjxl
        set(JPEGXL_BUNDLE_SKCMS OFF CACHE BOOL "build jpegxl bundle sjpeg" FORCE)
        # JPEGXL_ENABLE_OPENEXR variable is used by libjxl
        set(JPEGXL_ENABLE_OPENEXR OFF CACHE BOOL "build jpegxl openxr" FORCE)
        # JPEGXL_ENABLE_MANPAGES variable is used by libjxl
        set(JPEGXL_ENABLE_MANPAGES OFF CACHE BOOL "libjxl manpage option" FORCE)
        # JPEGXL_ENABLE_PLUGINS variable is used by libjxl
        set(JPEGXL_ENABLE_PLUGINS OFF CACHE BOOL "libjxl manpage option" FORCE)
        # JPEGXL_ENABLE_SKCMS variable is used by libjxl
        set(JPEGXL_ENABLE_SKCMS OFF CACHE BOOL "libjxl skcms option" FORCE)
        # JPEGXL_FORCE_SYSTEM_HWY variable is used by libjxl
        set(JPEGXL_FORCE_SYSTEM_HWY OFF CACHE BOOL "libjxl highway option" FORCE)
        sdl_check_project_in_subfolder(external/libjxl libjxl SDL2IMAGE_VENDORED)
        set(BUILD_SHARED_LIBS ${SDL2IMAGE_JXL_SHARED})
        add_subdirectory(external/libjxl EXCLUDE_FROM_ALL)
        if(BUILD_SHARED_LIBS)
            set(jxl_lib jxl)
            list(APPEND INSTALL_EXTRA_TARGETS brotlidec brotlicommon brotlienc hwy ${jxl_lib})
            if(NOT SDL2IMAGE_JXL_SHARED)
                list(APPEND PC_LIBS
                    -l$<TARGET_FILE_BASE_NAME:brotlidec> -l$<TARGET_FILE_BASE_NAME:brotlicommon>
                    -l$<TARGET_FILE_BASE_NAME:brotlienc> -l$<TARGET_FILE_BASE_NAME:${jxl_lib}>
                )
            endif()
        else()
            set(jxl_lib jxl_dec-static)
            list(APPEND INSTALL_EXTRA_TARGETS brotlidec brotlicommon hwy ${jxl_lib})
        endif()
        set_target_properties(${jxl_lib} PROPERTIES EXPORT_NAME external_libjxl)
        add_library(SDL2_image::external_libjxl ALIAS ${jxl_lib})
        if(NOT TARGET libjxl::libjxl)
            add_library(libjxl::libjxl ALIAS ${jxl_lib})
        endif()
    else()
        find_package(libjxl ${required})
        if(libjxl_FOUND)
            set(SDL2IMAGE_JXL_ENABLED TRUE)
            message(STATUS "${PROJECT_NAME}: Using system libjxl")
            if(NOT SDL2IMAGE_JXL_SHARED)
                list(APPEND PC_REQUIRES libjxl)
            endif()
        else()
            message(${fatal_error} "libjxl NOT found")
        endif()
    endif()
    if(SDL2IMAGE_JXL_ENABLED)
        target_compile_definitions(SDL2_image PRIVATE LOAD_JXL)
        if(SDL2IMAGE_JXL_SHARED)
            target_include_directories(SDL2_image PRIVATE
                $<TARGET_PROPERTY:libjxl::libjxl,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:libjxl::libjxl,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:libjxl::libjxl,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_jxl libjxl::libjxl)
            message(STATUS "Dynamic libjxl: ${dynamic_jxl}")
            target_compile_definitions(SDL2_image PRIVATE "LOAD_JXL_DYNAMIC=\"${dynamic_jxl}\"")
            if(SDL2IMAGE_JXL_VENDORED)
                add_dependencies(SDL2_image libjxl::libjxl)
            endif()
        else()
            target_link_libraries(SDL2_image PRIVATE libjxl::libjxl)
        endif()
    endif()
endif()

list(APPEND SDL2IMAGE_BACKENDS LBM)
set(SDL2IMAGE_LBM_ENABLED FALSE)
if(SDL2IMAGE_LBM)
    set(SDL2IMAGE_LBM_ENABLED TRUE)
    target_compile_definitions(SDL2_image PRIVATE LOAD_LBM)
endif()

list(APPEND SDL2IMAGE_BACKENDS PCX)
set(SDL2IMAGE_PCX_ENABLED FALSE)
if(SDL2IMAGE_PCX)
    set(SDL2IMAGE_PCX_ENABLED TRUE)
    target_compile_definitions(SDL2_image PRIVATE LOAD_PCX)
endif()

list(APPEND SDL2IMAGE_BACKENDS PNG)
set(SDL2IMAGE_PNG_ENABLED FALSE)
if(SDL2IMAGE_PNG)
    if(SDL2IMAGE_BACKEND_STB OR SDL2IMAGE_BACKEND_WIC OR SDL2IMAGE_BACKEND_IMAGEIO)
        set(SDL2IMAGE_PNG_ENABLED TRUE)
    else()
        if(SDL2IMAGE_PNG_VENDORED)
            set(SDL2IMAGE_PNG_ENABLED TRUE)
            message(STATUS "${PROJECT_NAME}: Using vendored libpng")
            sdl_check_project_in_subfolder(external/libpng libpng SDL2IMAGE_VENDORED)
            add_subdirectory(external/libpng EXCLUDE_FROM_ALL)
            if(SDL2IMAGE_PNG_SHARED)
                set(PNG_LIBRARY png_shared)
            else()
                set(PNG_LIBRARY png_static)
            endif()
            add_library(PNG::PNG ALIAS ${PNG_LIBRARY})
            target_include_directories(SDL2_image PRIVATE external/libpng)
            list(APPEND INSTALL_EXTRA_TARGETS ${PNG_LIBRARY})
            set_target_properties(${PNG_LIBRARY} PROPERTIES EXPORT_NAME external_libpng)
            add_library(SDL2_image::external_libpng ALIAS ${PNG_LIBRARY})
            if(NOT SDL2IMAGE_PNG_SHARED)
                list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:${PNG_LIBRARY}>)
                if(SDL2IMAGE_ZLIB_VENDORED)
                    list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:${ZLIB_LIBRARY}>)
                else()
                    list(APPEND PC_REQUIRES zlib)
                endif()
            endif()
        else()
            find_package(PNG ${required})
            if(PNG_FOUND)
                set(SDL2IMAGE_PNG_ENABLED TRUE)
                message(STATUS "${PROJECT_NAME}: Using system libpng")
                if(NOT SDL2IMAGE_PNG_SHARED)
                    list(APPEND PC_REQUIRES libpng)
                endif()
            else()
                message(${FATAL_ERROR} "libpng NOT found")
            endif()
        endif()
        if(SDL2IMAGE_PNG_ENABLED)
            if(SDL2IMAGE_PNG_SHARED)
                target_include_directories(SDL2_image PRIVATE
                    $<TARGET_PROPERTY:PNG::PNG,INTERFACE_INCLUDE_DIRECTORIES>
                )
                target_get_dynamic_library(dynamic_png PNG::PNG)
                message(STATUS "Dynamic libpng: ${dynamic_png}")
                target_compile_definitions(SDL2_image PRIVATE "LOAD_PNG_DYNAMIC=\"${dynamic_png}\"")
                if(SDL2IMAGE_PNG_VENDORED)
                    add_dependencies(SDL2_image PNG::PNG)
                endif()
            else()
                target_link_libraries(SDL2_image PRIVATE PNG::PNG)
            endif()
        endif()
    endif()
    if(SDL2IMAGE_PNG_ENABLED)
        target_compile_definitions(SDL2_image PRIVATE
            LOAD_PNG
            SDL_IMAGE_SAVE_PNG=$<BOOL:${SDL2IMAGE_PNG_SAVE}>
        )
    endif()
endif()

list(APPEND SDL2IMAGE_BACKENDS PNM)
set(SDL2IMAGE_PNM_ENABLED FALSE)
if(SDL2IMAGE_PNM)
    set(SDL2IMAGE_PNM_ENABLED TRUE)
    target_compile_definitions(SDL2_image PRIVATE LOAD_PNM)
endif()

list(APPEND SDL2IMAGE_BACKENDS QOI)
set(SDL2IMAGE_QOI_ENABLED FALSE)
if(SDL2IMAGE_QOI)
    set(SDL2IMAGE_QOI_ENABLED TRUE)
    target_compile_definitions(SDL2_image PRIVATE LOAD_QOI)
endif()

list(APPEND SDL2IMAGE_BACKENDS SVG)
set(SDL2IMAGE_SVG_ENABLED FALSE)
if(SDL2IMAGE_SVG)
    set(SDL2IMAGE_SVG_ENABLED TRUE)
    target_compile_definitions(SDL2_image PRIVATE LOAD_SVG)
endif()

list(APPEND SDL2IMAGE_BACKENDS TGA)
set(SDL2IMAGE_TGA_ENABLED FALSE)
if(SDL2IMAGE_TGA)
    set(SDL2IMAGE_TGA_ENABLED TRUE)
    target_compile_definitions(SDL2_image PRIVATE LOAD_TGA)
endif()

list(APPEND SDL2IMAGE_BACKENDS TIF)
set(SDL2IMAGE_TIF_ENABLED FALSE)
if(SDL2IMAGE_TIF)
    if(SDL2IMAGE_TIF_VENDORED)
        set(SDL2IMAGE_TIF_ENABLED TRUE)
        message(STATUS "${PROJECT_NAME}: Using vendored libtiff")
        # jpeg variable is used by vendored libtiff
        set(jpeg OFF CACHE BOOL "libtiff: jpeg option" FORCE)
        # libdeflate variable is used by vendored libtiff
        set(libdeflate OFF CACHE BOOL "libtiff: libdeflate option" FORCE)
        # DEFLATE_FOUND variable is used by vendored libtiff
        set(DEFLATE_FOUND OFF CACHE BOOL "libtiff: libdeflate option" FORCE)
        # zlib variable is used by vendored libtiff (controls use of `find_package`)
        set(zlib OFF CACHE BOOL "libtiff: find zlib using find_package" FORCE)
        # ZLIB_FOUND is used by vendored libtiff
        set(ZLIB_FOUND "")
        # lzma variable is used by vendored libtiff
        set(lzma OFF CACHE BOOL "libtiff: lzma option" FORCE)
        # webp variable is used by vendored libtiff
        set(webp OFF CACHE BOOL "libtiff: webp option" FORCE)
        # zstd variable is used by vendored libtiff
        set(zstd OFF CACHE BOOL "libtiff: zstd option" FORCE)
        # ZSTD_FOUND variable is used by vendored libtiff
        set(ZSTD_FOUND OFF)
        # WEBP_LIBRARY variable is used by vendored libtiff
        set(WEBP_LIBRARY "")
        sdl_check_project_in_subfolder(external/libtiff libtiff SDL2IMAGE_VENDORED)
        set(BUILD_SHARED_LIBS ${SDL2IMAGE_TIF_SHARED})
        add_subdirectory(external/libtiff EXCLUDE_FROM_ALL)
        add_library(TIFF::TIFF ALIAS tiff)
        set(SDL2IMAGE_TIF_TARGET "TIFF::TIFF")
        list(APPEND INSTALL_EXTRA_TARGETS tiff)
        set_target_properties(tiff PROPERTIES EXPORT_NAME external_libtiff)
        add_library(SDL2_image::external_libtiff ALIAS tiff)
        if(NOT SDL2IMAGE_TIF_SHARED)
            list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:tiff>)
        endif()
    else()
        find_package(TIFF ${required})
        if(TIFF_FOUND)
            set(SDL2IMAGE_TIF_ENABLED TRUE)
            message(STATUS "${PROJECT_NAME}: Using system libtiff")
            if(NOT SDL2IMAGE_TIF_SHARED)
                list(APPEND PC_REQUIRES libtiff-4)
            endif()
            if(TARGET TIFF::tiff)
                # Introduced in CMake 3.28
                # TIFF::TIFF still exists, but it is an INTERFACE library linking to TIFF::tiff (no ALIAS library)
                set(SDL2IMAGE_TIF_TARGET "TIFF::tiff")
            else()
                set(SDL2IMAGE_TIF_TARGET "TIFF::TIFF")
            endif()
        else()
            message(${fatal_error} "libtiff NOT found")
        endif()
    endif()
    if(SDL2IMAGE_TIF_ENABLED)
        target_compile_definitions(SDL2_image PRIVATE LOAD_TIF)
        if(SDL2IMAGE_TIF_SHARED)
            target_include_directories(SDL2_image PRIVATE
                $<TARGET_PROPERTY:${SDL2IMAGE_TIF_TARGET},INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:${SDL2IMAGE_TIF_TARGET},INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:${SDL2IMAGE_TIF_TARGET},INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_tif ${SDL2IMAGE_TIF_TARGET})
            message(STATUS "Dynamic libtiff: ${dynamic_tif}")
            target_compile_definitions(SDL2_image PRIVATE "LOAD_TIF_DYNAMIC=\"${dynamic_tif}\"")
            if(SDL2IMAGE_TIF_VENDORED)
                add_dependencies(SDL2_image ${SDL2IMAGE_TIF_TARGET})
            endif()
        else()
            target_link_libraries(SDL2_image PRIVATE ${SDL2IMAGE_TIF_TARGET})
        endif()
    endif()
endif()

list(APPEND SDL2IMAGE_BACKENDS WEBP)
set(SDL2IMAGE_WEBP_ENABLED FALSE)
if(SDL2IMAGE_WEBP)
    # missing cpufeatures
    if(SDL2IMAGE_WEBP_VENDORED)
        set(SDL2IMAGE_WEBP_ENABLED TRUE)
        message(STATUS "${PROJECT_NAME}: Using vendored libwebp")
        sdl_check_project_in_subfolder(external/libwebp libwebp SDL2IMAGE_VENDORED)
        set(BUILD_SHARED_LIBS ${SDL2IMAGE_WEBP_SHARED})
        add_subdirectory(external/libwebp EXCLUDE_FROM_ALL)
        target_include_directories(SDL2_image PRIVATE external/libwebp/src)
        add_library(WebP::webp ALIAS webp)
        add_library(WebP::webpdemux ALIAS webpdemux)
        list(APPEND INSTALL_EXTRA_TARGETS webp webpdemux)
        set_target_properties(webp PROPERTIES EXPORT_NAME "external_libwebp")
        add_library(SDL2_image::external_libwebp ALIAS webp)
    else()
        find_package(webp ${required})
        if(webp_FOUND)
            set(SDL2IMAGE_WEBP_ENABLED TRUE)
            message(STATUS "${PROJECT_NAME}: Using system libwebp")
            if(NOT SDL2IMAGE_WEBP_SHARED)
                list(APPEND PC_REQUIRES libwebp libwebpdemux)
            endif()
        else()
            message(${fatal_error} "libwebp NOT found")
        endif()
    endif()
    if(SDL2IMAGE_WEBP_ENABLED)
        target_compile_definitions(SDL2_image PRIVATE LOAD_WEBP)
        if(SDL2IMAGE_WEBP_SHARED)
            target_include_directories(SDL2_image PRIVATE
                $<TARGET_PROPERTY:WebP::webp,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:WebP::webp,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:WebP::webp,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:WebP::webpdemux,INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:WebP::webpdemux,INTERFACE_INCLUDE_DIRECTORIES>
                $<TARGET_PROPERTY:WebP::webpdemux,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
            )
            target_get_dynamic_library(dynamic_webpdemux WebP::webpdemux)
            message(STATUS "Dynamic libwebpdemux: ${dynamic_webpdemux}")
            target_compile_definitions(SDL2_image PRIVATE "LOAD_WEBPDEMUX_DYNAMIC=\"${dynamic_webpdemux}\"")
            target_get_dynamic_library(dynamic_webp WebP::webp)
            message(STATUS "Dynamic libwebp: ${dynamic_webp}")
            target_compile_definitions(SDL2_image PRIVATE "LOAD_WEBP_DYNAMIC=\"${dynamic_webp}\"")
            if(SDL2IMAGE_WEBP_VENDORED)
                add_dependencies(SDL2_image WebP::webp WebP::webpdemux)
            endif()
        else()
            target_link_libraries(SDL2_image PRIVATE WebP::webp WebP::webpdemux)
        endif()
    endif()
endif()

list(APPEND SDL2IMAGE_BACKENDS XCF)
set(SDL2IMAGE_XCF_ENABLED FALSE)
if(SDL2IMAGE_XCF)
    set(SDL2IMAGE_XCF_ENABLED TRUE)
    target_compile_definitions(SDL2_image PRIVATE LOAD_XCF)
endif()

list(APPEND SDL2IMAGE_BACKENDS XPM)
set(SDL2IMAGE_XPM_ENABLED FALSE)
if(SDL2IMAGE_XPM)
    set(SDL2IMAGE_XPM_ENABLED TRUE)
    target_compile_definitions(SDL2_image PRIVATE LOAD_XPM)
endif()

list(APPEND SDL2IMAGE_BACKENDS XV)
set(SDL2IMAGE_XV_ENABLED FALSE)
if(SDL2IMAGE_XV)
    set(SDL2IMAGE_XV_ENABLED TRUE)
    target_compile_definitions(SDL2_image PRIVATE LOAD_XV)
endif()

# Restore BUILD_SHARED_LIBS
set(BUILD_SHARED_LIBS ${SDL2IMAGE_BUILD_SHARED_LIBS})

if(SDL2IMAGE_INSTALL)
    install(
        TARGETS SDL2_image
        EXPORT SDL2ImageExports
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT devel
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT library
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT library
    )
    install(
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_image.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/SDL2" COMPONENT DEVEL
    )

    if(INSTALL_EXTRA_TARGETS)
        install(
            TARGETS ${INSTALL_EXTRA_TARGETS}
            EXPORT SDL2ImageExports
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT devel
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT library
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT library
            PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" COMPONENT devel
        )
    endif()

    if(WIN32 AND NOT MINGW)
        set(SDLIMAGE_INSTALL_CMAKEDIR_DEFAULT "cmake")
    else()
        set(SDLIMAGE_INSTALL_CMAKEDIR_DEFAULT "${CMAKE_INSTALL_LIBDIR}/cmake/SDL2_image")
    endif()
    set(SDLIMAGE_INSTALL_CMAKEDIR "${SDLIMAGE_INSTALL_CMAKEDIR_DEFAULT}" CACHE STRING "Location where to install SDL2_imageConfig.cmake")

    configure_package_config_file(SDL2_imageConfig.cmake.in SDL2_imageConfig.cmake
        INSTALL_DESTINATION "${SDLIMAGE_INSTALL_CMAKEDIR}"
    )
    write_basic_package_version_file("${PROJECT_BINARY_DIR}/SDL2_imageConfigVersion.cmake"
        VERSION ${FULL_VERSION}
        COMPATIBILITY AnyNewerVersion
    )
    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/SDL2_imageConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/SDL2_imageConfigVersion.cmake"
            cmake/Findlibjxl.cmake
            cmake/Findwebp.cmake
        DESTINATION "${SDLIMAGE_INSTALL_CMAKEDIR}"
        COMPONENT devel
    )
    install(EXPORT SDL2ImageExports
        FILE SDL2_image-${sdl2_image_install_name_infix}-targets.cmake
        NAMESPACE SDL2_image::
        DESTINATION "${SDLIMAGE_INSTALL_CMAKEDIR}"
        COMPONENT devel
    )

    set(prefix "${CMAKE_INSTALL_PREFIX}")
    set(exec_prefix "\${prefix}")
    set(libdir "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
    set(includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
    set(PACKAGE "${PROJECT_NAME}")
    set(VERSION "${FULL_VERSION}")
    set(SDL_VERSION "${SDL_REQUIRED_VERSION}")
    string(JOIN " " PC_REQUIRES ${PC_REQUIRES})
    string(JOIN " " PC_LIBS ${PC_LIBS})
    configure_file("${PROJECT_SOURCE_DIR}/SDL2_image.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/SDL2_image.pc.intermediate" @ONLY)
    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/SDL2_image-$<CONFIG>.pc" INPUT "${CMAKE_CURRENT_BINARY_DIR}/SDL2_image.pc.intermediate")

    set(PC_DESTDIR)
    if(CMAKE_SYSTEM_NAME MATCHES FreeBSD)
        # FreeBSD uses ${PREFIX}/libdata/pkgconfig
        set(PC_DESTDIR "libdata/pkgconfig")
    else()
        set(PC_DESTDIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
    endif()
    # Always install SDL2_net.pc file: libraries might be different between config modes
    install(CODE "
            # FIXME: use file(COPY_FILE) if minimum CMake version >= 3.21
            execute_process(COMMAND \"\${CMAKE_COMMAND}\" -E copy_if_different
                \"${CMAKE_CURRENT_BINARY_DIR}/SDL2_image-$<CONFIG>.pc\"
                \"${CMAKE_CURRENT_BINARY_DIR}/SDL2_image.pc\")
            file(INSTALL DESTINATION \"\${CMAKE_INSTALL_PREFIX}/${PC_DESTDIR}\"
                TYPE FILE
                FILES \"${CMAKE_CURRENT_BINARY_DIR}/SDL2_image.pc\")" COMPONENT devel)

    if(SDL2IMAGE_BUILD_SHARED_LIBS AND (APPLE OR (UNIX AND NOT ANDROID)))
        install(
            FILES
                "${PROJECT_BINARY_DIR}/libSDL2_image$<$<CONFIG:Debug>:${SDL2IMAGE_DEBUG_POSTFIX}>$<TARGET_FILE_SUFFIX:SDL2_image>"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            COMPONENT devel
        )
    endif()

    install(FILES "LICENSE.txt"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/licenses/${PROJECT_NAME}"
        COMPONENT library
    )
endif()

if(SDL2IMAGE_SAMPLES)
    add_executable(showanim examples/showanim.c)
    add_executable(showimage examples/showimage.c)

    find_package(SDL2main)

    foreach(prog showanim showimage)
        # FIXME: mingw should be handled by SDL2::SDL2(-static) target
        if(MINGW)
            target_link_libraries(${prog} PRIVATE mingw32)
            target_link_options(${prog} PRIVATE -mwindows)
        endif()
        target_link_libraries(${prog} PRIVATE SDL2_image::${sdl2_image_export_name})
        if(TARGET SDL2::SDL2main)
            target_link_libraries(${prog} PRIVATE SDL2::SDL2main)
        endif()
        target_link_libraries(${prog} PRIVATE ${sdl2_target_name})

        if(SDL2IMAGE_SAMPLES_INSTALL)
            install(TARGETS ${prog}
                RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            )
        endif()
    endforeach()
endif()

add_library(SDL2::image INTERFACE IMPORTED GLOBAL)
set_target_properties(SDL2::image PROPERTIES
    INTERFACE_LINK_LIBRARIES "SDL2_image"
)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.17")
    set_target_properties(SDL2::image PROPERTIES
        DEPRECATION "Use SDL2_image::SDL2_image or SDL2_image::SDL2_image-static instead"
    )
endif()

if(SDL2IMAGE_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()


set(available_deps)
set(unavailable_deps)
foreach(dep IN LISTS SDL2IMAGE_BACKENDS)
    set(var SDL2IMAGE_${dep}_ENABLED)
    if(NOT DEFINED ${var})
        message(AUTHOR_WARNING "${var} not defined")
    endif()
    if(${var})
        list(APPEND available_deps ${dep})
    else()
        list(APPEND unavailable_deps ${dep})
    endif()
endforeach()
string(JOIN " " avail_str ${available_deps})
string(TOLOWER "${avail_str}" avail_str)
string(JOIN " " unavail_str ${unavailable_deps})
string(TOLOWER "${unavail_str}" unavail_str)
message(STATUS "SDL2_image backends:")
message(STATUS "- enabled:  ${avail_str}")
message(STATUS "- disabled: ${unavail_str}")
