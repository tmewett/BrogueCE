Brogue CE
=========

Brogue: Community Edition is a codebase for the roguelike
[Brogue](https://sites.google.com/site/broguegame/) by Brian Walker. It has
several goals:

- add community-accepted bugfixes and game changes
- provide a cross-platform build system
- be a convenient base for forks and ports to new platforms

## Building brogue

### Linux

1. Edit `config.mk` to configure the build, and install any necessary
dependencies it lists for your configuration

2. If you've set `GRAPHICS=YES`, copy (or symlink) your built `libtcod.so`
to `bin/libtcod.so.1`

3. Run `make bin/brogue`

4. Run the game with the `./brogue` script

### Windows

Building on Windows requires a Linux-like build environment: you'll need
GNU Make and a C compiler such as MinGW.

The rest of this guide assumes you have installed MinGW and the MinGW
binaries for make and gcc are in your PATH. It also assumes the
Windows command line or PowerShell are used to input commands.

Edit `config.mk` to configure the build, and install and build any
necessary dependencies it lists for your configuration.

#### libtcod (GRAPHICS=YES)

1. Build tcod.dll - see the README.md within the libtcod subdirectory
for how to build libtcod on Windows. This includes extracting SDL and
it is assumed in this guide that SDL has been extracted in the
location specified by libtcod\README.md.

2. Build brogue as follows:

```
mingw32-make.exe CC=gcc C_INCLUDE_PATH="libtcod\SDL-1.2.15\include\SDL" LIBRARY_PATH="libtcod\SDL-1.2.15\lib" SDL_CONFIG="libtcod\SDL-1.2.15\bin\sdl-config"
```

(if SDL is extracted elsewhere, alter the paths above)

3. Copy tcod.dll to the bin directory:

```
cp .\libtcod\tcod.dll bin
```

4. Copy SDL.dll from the SDL extract to bin:

```
cp .\libtcod\SDL-1.2.15\bin\SDL.dll bin
```

5. Launch brogue.exe!

```
cd .\bin
brogue.exe
```

(note brogue.exe requires the DLLs in the same directory, so you must
launch from the bin directory)