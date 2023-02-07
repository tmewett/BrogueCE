include config.mk

cflags := -Isrc/brogue -Isrc/platform -std=c99 \
	-Wall -Wpedantic -Werror=implicit -Wno-parentheses -Wno-unused-result \
	-Wformat -Werror=format-security -Wformat-overflow=0
libs := -lm
cppflags := -DDATADIR=$(DATADIR)

sources := $(wildcard src/brogue/*.c) $(addprefix src/platform/,main.c platformdependent.c)

ifeq ($(RELEASE),YES)
	extra_version :=
else
	extra_version := $(shell bash tools/git-extra-version)
endif
cppflags += -DBROGUE_EXTRA_VERSION='"$(extra_version)"'

ifeq ($(TERMINAL),YES)
	sources += $(addprefix src/platform/,curses-platform.c term.c)
	cppflags += -DBROGUE_CURSES
	libs += -lncurses
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

ifeq ($(MAC_APP),YES)
	cppflags += -DSDL_PATHS
endif

ifeq ($(DEBUG),YES)
	cflags += -g -Og
	cppflags += -DENABLE_PLAYBACK_SWITCH
else
	cflags += -O2
endif

# Add user-provided flags.
cflags += $(CFLAGS)
cppflags += $(CPPFLAGS)
libs += $(LDLIBS)

objects := $(sources:.c=.o)

include recipes/*.mk

clean:
	$(warning 'make clean' is no longer needed in many situations, so is not supported. Use 'make -B' to force rebuild something.)

escape = $(subst ','\'',$(1))
vars:
	mkdir vars
# Write the value to a temporary file and only overwrite if it's different.
vars/%: vars FORCE
	@echo '$(call escape,$($*))' > vars/$*.tmp
	@if cmp --quiet vars/$*.tmp vars/$*; then :; else cp vars/$*.tmp vars/$*; fi


FORCE:
