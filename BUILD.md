Building Brogue CE
==================

You need these things to compile the game:

- a C compiler (Clang or GCC)
- Make
- anything extra listed in `config.mk` that you require

Once you have those, the game is built by running `make`. You can configure
the build by editing `config.mk` or appending options to the Make command,
e.g. `make TERMINAL=YES`.

More specific instructions follow on how to acquire the dependencies and
build the game.


Windows
-------

There are many ways to compile software on Windows. The method here was
tested by the author and is relatively simple.

### Setting up

1. Install [MSYS2][1] for x86_64. Refer to [its wiki][2] for essential info
on how to use it.

2. (Installing the dependencies) Open the MSYS2 shell and run the following:

    ```
    pacman -S make mingw-w64-x86_64-{gcc,SDL2,SDL2_image}
    ```

### Building

1. (Compiling the game) Open the Mingw64 shell. Navigate to the Brogue CE
directory (the one containing this file; your Windows drives can be accessed
at `/c`, `/d` etc.) and run

    ```
    make bin/brogue.exe
    ```

2. Optional: You can now run the game by running `cd bin; ./brogue.exe`.
However if you want to be able to run it from outside the Mingw64 shell,
you'll need to add the `/mingw64/bin` directory of MSYS2 to your system
PATH. By default this is at `C:\msys2\mingw64\bin`. Some info is here:
[How do I set or change the PATH system variable?][3] You only need to
perform this step once.


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
    make bin/brogue
    ```


[1]: https://www.msys2.org/
[2]: https://github.com/msys2/msys2/wiki
[3]: https://www.java.com/en/download/help/path.xml
