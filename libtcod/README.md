This is a modified and stripped-down distribution of libtcod 1.5.2 as
distributed with Brogue.

## Installation

You'll need SDL 1.2.

### Linux

1. Run `make libtcod.so`

### Windows

Building on Windows requires a Linux-like build environment: you'll need
GNU Make and a C compiler such as MinGW.

1. If everything is installed in the places expected (PATH, etc.),
you can just run `make tcod.dll`. However there are various variables
you can set, like `CC` and `LDFLAGS`, which you can use to inform the
build process about your environment. For example, when cross-compiling
from Linux, you might run something like `make CC=x86_64-w64-mingw32-gcc
SDL_CONFIG=/windows/bin/sdl-config tcod.dll`
