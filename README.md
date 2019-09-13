Brogue CE
=========

Brogue: Community Edition is a codebase for the roguelike
[Brogue](https://sites.google.com/site/broguegame/) by Brian Walker. It has
several goals:

- add community-accepted bugfixes and game changes
- provide a cross-platform build system
- be a convenient base for forks and ports to new platforms

## Installation

Building has only been tested on Linux so far. The build system will soon
support other OSes cleanly. If you have experience with C compilation it
shouldn't be too hard to get it to build with mingw-w64 on Windows.

1. Edit `config.mk` to configure the build
1. Install necessary dependencies
1. Run `make`

The game can then be run via the `./brogue` script. If you've built with
`GRAPHICS=yes` you'll need to also copy (or symlink) your built `libtcod.so`
to `bin/libtcod.so.1`.
