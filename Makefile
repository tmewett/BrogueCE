include config.mk

base_cflags := -Isrc/brogue -Isrc/platform -Isrc/variants -std=c99 \
	-Wall -Wpedantic -Werror=implicit -Wno-parentheses -Wno-unused-result \
	-Wformat -Werror=format-security -Wformat-overflow=0 -Wmissing-prototypes
base_libs := -lm
base_cppflags := -DDATADIR=$(DATADIR)
cflags = $(base_cflags)
libs = $(base_libs)
cppflags = $(base_cppflags)
bridge_cflags = $(base_cflags)
bridge_libs = $(base_libs)
bridge_cppflags = $(base_cppflags)
uname_s := $(shell uname -s)

sources := $(wildcard src/brogue/*.c) $(wildcard src/variants/*.c) $(addprefix src/platform/,main.c platformdependent.c null-platform.c)
objects :=
bridge_sources := $(wildcard src/brogue/*.c) $(wildcard src/variants/*.c) $(addprefix src/platform/,platformdependent.c null-platform.c bridge-platform.c)

ifeq ($(uname_s),Darwin)
bridge_library := bin/libbruhogue_brogue.dylib
bridge_ldflags := -dynamiclib
else
bridge_library := bin/libbruhogue_brogue.so
bridge_ldflags := -shared
endif

ifeq ($(SYSTEM),WINDOWS)
objects += windows/resources.o
.exe := .exe
endif

ifeq ($(SYSTEM),OS2)
cflags += -march=pentium4 -mtune=pentium4 -Zmt -Wno-narrowing
cppflags += -D__ST_MT_ERRNO__
libs += -lcx -lc -Zomf -Zbin-files -Zargs-wild -Zargs-resp -Zhigh-mem -Zstack 8000
objects += os2/icon.res os2/brogue.lib
.exe := .exe
endif

ifeq ($(RELEASE),YES)
extra_version :=
else
extra_version := $(shell bash tools/git-extra-version)
endif

ifeq ($(TERMINAL),YES)
sources += $(addprefix src/platform/,curses-platform.c term.c)
cppflags += -DBROGUE_CURSES
libs += -lncurses
ifeq ($(SYSTEM),OS2)
libs += -ltinfo
endif
endif

ifeq ($(GRAPHICS),YES)
sources += $(addprefix src/platform/,sdl2-platform.c tiles.c)
cflags += $(shell $(SDL_CONFIG) --cflags)
cppflags += -DBROGUE_SDL
libs += $(shell $(SDL_CONFIG) --libs) -lSDL2_image
endif

ifeq ($(WEBBROGUE),YES)
sources += $(addprefix src/platform/,web-platform.c)
cppflags += -DBROGUE_WEB
endif

ifeq ($(HEADLESS),YES)
sources += $(addprefix src/platform/,headless-platform.c)
cppflags += -DBROGUE_HEADLESS
endif

ifeq ($(RAPIDBROGUE),YES)
cppflags += -DRAPID_BROGUE
endif

ifeq ($(MAC_APP),YES)
cppflags += -DSDL_PATHS
endif

ifeq ($(DEBUG),YES)
cflags += -g -Og
bridge_cflags += -g -Og
cppflags += -DENABLE_PLAYBACK_SWITCH
bridge_cppflags += -DENABLE_PLAYBACK_SWITCH
else
cflags += -O2
bridge_cflags += -O2
endif

# Add user-provided flags.
cflags += $(CFLAGS)
cppflags += $(CPPFLAGS)
libs += $(LDLIBS)
bridge_cflags += $(CFLAGS)
bridge_cppflags += $(CPPFLAGS)
bridge_libs += $(LDLIBS)

objects += $(sources:.c=.o)

include make/*.mk
.DEFAULT_GOAL := bin/brogue$(.exe)

.PHONY: bridge
bridge: $(bridge_library)

$(bridge_library): $(bridge_sources) src/brogue/Rogue.h src/brogue/Globals.h src/brogue/GlobalsBase.h vars/bridge_cppflags vars/bridge_cflags vars/bridge_libs vars/extra_version Makefile
	mkdir -p bin
	$(CC) $(bridge_cppflags) -DBROGUE_BRIDGE -DBROGUE_EXTRA_VERSION='"$(extra_version)"' $(bridge_cflags) -fPIC $(bridge_ldflags) -o $@ $(bridge_sources) $(bridge_libs) -lpthread

clean:
	$(warning 'make clean' is no longer needed in many situations, so is not supported. Use 'make -B' to force rebuild something.)

escape = $(subst ','\'',$(1))
vars:
	mkdir -p vars
# Write the value to a temporary file and only overwrite if it's different.
vars/%: vars FORCE
	@echo '$(call escape,$($*))' > vars/$*.tmp
	@if ! cmp --quiet vars/$*.tmp vars/$*; then cp vars/$*.tmp vars/$*; fi


FORCE:
