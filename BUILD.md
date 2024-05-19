Building Brogue CE
==================

You need these things at minimum to compile the game:

- a C compiler (Clang or GCC)
- Make
- diffutils (cmp)

You will also need additional dependencies based on the build options you select.
You can see the options and their requirements in `config.mk`. You can configure
the build by editing `config.mk` or appending options to the Make command,
e.g. `make TERMINAL=YES`.

More specific instructions follow on how to acquire the dependencies and
build the game.


Windows
-------

There are many ways to compile software on Windows. The method here was
tested by the author and is relatively simple.

### Setting up

1. Install [MSYS2][1] for x86_64. Refer to the website for essential info
on how to use it.

2. (Installing the dependencies) Open the MSYS2 shell and run the following:

    ```
    pacman -S make diffutils mingw-w64-x86_64-{gcc,SDL2,SDL2_image}
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


Mac
---

These instructions are intended for developers and testers, as this will
not create an App. You will have to run the game with the `./brogue` script,
just like for the Linux version.

### Setting up

1. Install the [Homebrew package manager][4].

2. Install the dependencies:

    ```
    brew install sdl2 sdl2_image
    ```

### Building

1. Navigate to the Brogue CE directory (the one containing this file) and run:

    ```
    make bin/brogue
    ```

### App distribution

The above steps will suffice for single-user testing, but to create an app
bundle instead of a loose binary:

1.  (`make clean` if necessary)
1.  `make GRAPHICS=YES MAC_APP=YES Brogue.app`
1.  Rename to "Brogue CE.app"

To distribute this app you will have to bundle the dylib libraries, e.g. with
[dylibbundler][]. See our GitHub Actions workflow for more details.

[dylibbundler]: https://github.com/auriamg/macdylibbundler


Linux
-----

These instructions are for the graphical version only.

### Setting up

1. Install the dependencies; for example, on Debian/Ubuntu, run

    ```
    sudo apt install make gcc diffutils libsdl2-2.0-0 libsdl2-dev libsdl2-image-2.0-0 libsdl2-image-dev
    ```

### Building

1. Navigate to where this file is and run

    ```
    make bin/brogue
    ```


[1]: https://www.msys2.org/
[3]: https://www.java.com/en/download/help/path.xml
[4]: https://brew.sh/
