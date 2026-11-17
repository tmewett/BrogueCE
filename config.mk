# Where to look for game data files (found in 'bin'). Must be without trailing slashes!
DATADIR := .

# Host and target platform. Values: LINUXLIKE, WINDOWS, OS2
SYSTEM := LINUXLIKE

# Include terminal support. Requires ncurses
TERMINAL := NO

# Include graphical support. Requires SDL3 and SDL3_image
#     brew install sdl3 sdl3_image pkg-config
GRAPHICS := YES
SDL_CONFIG := pkg-config sdl3

# Select web brogue mode. Requires POSIX system.
WEBBROGUE := NO

# Enable debugging mode. See top of Rogue.h for features
DEBUG := NO

# Declare this is a release build
RELEASE := NO

# Configure the executable to run from a macOS .app bundle (only works in graphical mode)
MAC_APP := NO
