enable_testing()

if(NOT TARGET SDL2::SDL2main)
    find_package(SDL2main)
endif()

if(NOT TARGET SDL2::SDL2test)
    find_package(SDL2test REQUIRED)
endif()

add_executable(testimage main.c)

set(ALL_TESTS
    testimage
)
set(RESOURCE_FILES
    palette.bmp
    palette.gif
    sample.avif
    sample.bmp
    sample.cur
    sample.ico
    sample.jpg
    sample.jxl
    sample.pcx
    sample.png
    sample.pnm
    sample.qoi
    sample.tga
    sample.tif
    sample.webp
    sample.xcf
    sample.xpm
    svg-class.bmp
    svg-class.svg
    svg.bmp
    svg.svg
    svg64.bmp
)

set(TESTS_ENVIRONMENT
    "SDL_TEST_SRCDIR=${CMAKE_CURRENT_SOURCE_DIR}"
    "SDL_TEST_BUILDDIR=${CMAKE_CURRENT_BINARY_DIR}"
    "SDL_VIDEODRIVER=dummy"
)

foreach(prog ${ALL_TESTS})
    target_compile_definitions(${prog} PRIVATE $<TARGET_PROPERTY:SDL2_image,COMPILE_DEFINITIONS>)
    target_link_libraries(${prog} PRIVATE SDL2_image::${sdl2_image_export_name})
    if(TARGET SDL2::SDL2main)
        target_link_libraries(${prog} PRIVATE SDL2::SDL2main)
    endif()
    target_link_libraries(${prog} PRIVATE SDL2::SDL2test ${sdl2_target_name})

    add_test(
        NAME ${prog}
        COMMAND ${prog}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_tests_properties(
        ${prog}
        PROPERTIES
        ENVIRONMENT "${TESTS_ENVIRONMENT}"
        TIMEOUT 30
    )
    if(SDL2IMAGE_TESTS_INSTALL)
        set(exe ${prog})
        set(installedtestsdir "${CMAKE_INSTALL_FULL_LIBEXECDIR}/installed-tests/${PROJECT_NAME}")
        configure_file(template.test.in "${exe}.test" @ONLY)
        install(
            FILES "${CMAKE_CURRENT_BINARY_DIR}/${exe}.test"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/installed-tests/${PROJECT_NAME}"
        )
    endif()
endforeach()

if(SDL2IMAGE_TESTS_INSTALL)
    install(
        TARGETS ${ALL_TESTS}
        DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}/installed-tests/${PROJECT_NAME}"
    )
    install(
        FILES ${RESOURCE_FILES}
        DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}/installed-tests/${PROJECT_NAME}"
    )
endif()
