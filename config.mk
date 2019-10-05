# Include terminal support. Requires ncurses
TERMINAL := YES

# Include graphical support. Requires SDL 1 and building libtcod
GRAPHICS := NO
# Path to sdl-config script
SDL_CONFIG := sdl-config
# Path of built libtcod source directory
# libtcod-1.5.2 can be copied from any recent release of Brogue
TCOD_DIR := libtcod-1.5.2

# Enable debugging mode. See top of Rogue.h for features
DEBUG := NO
