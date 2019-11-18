Building Brogue CE
==================

You need these things to compile the game:

- a C compiler (Clang or GCC)
- Make
- anything extra listed in `config.mk` that you require

Once you have those, the game is built by running `make`. You can configure
the build by editing `config.mk` or appending options to the Make command,
e.g. `make GRAPHICS=YES`.

More specific instructions follow.


Windows
-------

There are many ways to compile software on Windows. The method here was
tested by the author and is relatively simple. It only compiles the graphical
version.

### Setting up

1. Install [MSYS2](https://www.msys2.org/) for x86_64

2. Install the dependencies in MSYS2; open the MSYS2 shell and run the following

    ```
    pacman -S make mingw64-w64-x86_64-{gcc,SDL2,SDL2_image}
    ```

3. Copy required libraries to `bin`; navigate to where this file is (your
Windows drives can be accessed at `/c`, `/d` etc.) and run

    ```
    cp -t bin /mingw64/bin/{libjpeg-8.dll,liblzma-5.dll,libpng16-16.dll,libtiff-5.dll,libwebp-7.dll,libzstd.dll,SDL2.dll,SDL2_image.dll,zlib1.dll}
    ```

### Building

1. Open the Mingw64 shell, navigate to where this file is (your Windows
drives can be accessed at `/c`, `/d` etc.) and run

    ```
    make TERMINAL=NO GRAPHICS=YES bin/brogue.exe
    ```

Linux
-----

These instructions are for the graphical version only.

### Setting up

1. Install the dependencies; for example, on Debian/Ubuntu, run

    ```
    sudo apt install make gcc libsdl2-2.0-0 libsdl2-dev libsdl2-image-2.0-0 libsdl2-image-dev
    ```

### Building

1. Navigate to where this file is and run

    ```
    make TERMINAL=NO GRAPHICS=YES bin/brogue
    ```
