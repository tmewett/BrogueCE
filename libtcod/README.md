This is a modified and stripped-down distribution of libtcod 1.5.2 as
distributed with Brogue.

## Installation

You'll need SDL 1.2.

### Linux

1. Run `make libtcod.so`

### Windows

Building on Windows requires a Linux-like build environment: you'll
need GNU Make and a C compiler such as MinGW.

The rest of this guide assumes you have installed MinGW and the MinGW
binaries for make and gcc are in your PATH. It also assumes the
Windows command line or PowerShell are used to input commands.

#### SDL 1.2 dependency

You can obtain the development library for SDL 1.2
(SDL-devel-1.2.15-mingw32.tar.gz) from
https://www.libsdl.org/download-1.2.php.

The following instructions assume that the library is unzipped in this
directory (libtcod), e.g. so that the files are within
libtcod\SDL-1.2.15

If SDL is unzipped elsewhere, alter the relevant paths. We do not
install the SDL libraries and headers, instead referring to the
relevant paths when we invoke make.

#### Building tcod.dll

Build tcod.dll as follows:

```
mingw32-make.exe CC=gcc C_INCLUDE_PATH="SDL-1.2.15\include\SDL" LIBRARY_PATH="SDL-1.2.15\lib" SDL_CONFIG="SDL-1.2.15\bin\sdl-config" tcod.dll
```