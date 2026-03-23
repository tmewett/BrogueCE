`SDL2_image` automated test
===========================

Build-time tests
----------------

Configure with `--enable-tests` (Autotools) or `-DSDL2IMAGE_TESTS` (CMake).

Run build-time tests in the usual way, for example `make check`
(Autotools), or `ctest` or `make test` (CMake).

"As-installed" tests
--------------------

The tests can be installed alongside the library, allowing them to be
run on a target platform as a somewhat realistic equivalent of how a
game would behave.

Configure with `--enable-tests --enable-installed-tests` (Autotools) or
`-DSDL2IMAGE_TESTS -DSDL2IMAGE_TESTS_INSTALL` (CMake).

The tests and their required resource files are installed into
`${libexecdir}/installed-tests/SDL2_image`. When run, they will write
to the current working directory: create a temporary directory and run
them from there if necessary.

On platforms where `SDL_GetBasePath()` returns the directory containing
the executable, the whole `installed-tests/SDL2_image` directory can be
copied to any location and `testimage` can be run from there.

Metadata describing the tests is installed in
`${datadir}/installed-tests/SDL2_image`. This can be used to run the
tests with `ginsttest-runner` from
[gnome-desktop-testing](https://gitlab.gnome.org/GNOME/gnome-desktop-testing),
or any implementation of the same
[specification](https://wiki.gnome.org/Initiatives/GnomeGoals/InstalledTests).

Asserting format support
------------------------

By default, for each format that was configured at build-time, the test
asserts that the format can be loaded or saved as appropriate. Formats
that were not enabled at build-time are not tested.

This automatic behaviour is not always desirable: for example, if there
was a build-system regression that accidentally disabled WEBP support,
or if the required WEBP library was missing or not detected at build
time, then the test would would report WEBP as unsupported but would
not fail.

To check that the intended formats are actually supported, distributors
can set some environment variables before running the tests:

* For each *format* where loading can be supported, if
    `SDL_IMAGE_TEST_REQUIRE_LOAD_` + *format* is set to 1,
    the test will fail unless `SDL2_image` can load *format*.
    (For example, `export SDL_IMAGE_TEST_REQUIRE_LOAD_WEBP=1`.)

* For each *format* where saving can be supported (`JPG` or `PNG`), if
    `SDL_IMAGE_TEST_REQUIRE_SAVE_` + *format* is set to 1,
    the test will fail unless `SDL2_image` can load *format*.
    (For example, `export SDL_IMAGE_TEST_REQUIRE_SAVE_JPG=1`.)

Setting these variables to 0 causes them to be ignored, returning to the
automatic behaviour where the formats that were enabled at build-time
will be tested.

These variables are not set automatically for supported formats by the
Autotools or CMake build system, because that would defeat their purpose
of detecting build-system bugs.

The abbreviation used for *format* is the same as in the `supported[]`
table in `IMG.c`, for example `JPG` or `TIF`. `JPEG` and `TIFF` are
ignored.

On operating systems where environment variables are case-sensitive,
*format* must be in upper-case.
