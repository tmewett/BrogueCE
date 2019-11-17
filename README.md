Brogue CE
=========

Brogue: Community Edition is a codebase for the roguelike
[Brogue](https://sites.google.com/site/broguegame/) by Brian Walker. It has
several goals:

- add community-accepted bugfixes and game changes
- provide a cross-platform build system
- be a convenient base for forks and ports to new platforms

## Installation

### Linux

1. Edit `config.mk` to configure the build, and install any necessary
dependencies it lists for your configuration

2. Run `make bin/brogue`

3. Run the game with the `./brogue` script

### Windows

Building on Windows requires a Linux-like build environment: you'll need
GNU Make and a C compiler such as MinGW.

1. Edit `config.mk` to configure the build, and install any necessary
dependencies it lists for your configuration

2. If you've set `GRAPHICS=YES`, copy `SDL2.dll` and `SDL2_image.dll` to `bin`

3. If everything is installed in the places expected (PATH, etc.), you can
just run `make bin/brogue.exe`. However there are various variables you
can set, like `CC` and `LDFLAGS`, which you can use to inform the build
process about your environment. For example, when cross-compiling Brogue
from Linux, you might run something like `make CC=x86_64-w64-mingw32-gcc
LDFLAGS="-L/windows/lib" bin/brogue.exe`

4. Open the compiled exe
