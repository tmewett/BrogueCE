include config.mk

TCOD_DIR := libtcod

cflags := -Isrc/brogue -Isrc/platform -std=c99 -Wall -Wpedantic -Wno-parentheses -Wno-format-overflow
libs := -lm

sources := $(wildcard src/brogue/*.c) $(wildcard src/platform/*.c)
objects := $(sources:.c=.o)

ifeq ($(TERMINAL),YES)
	cppflags += -DBROGUE_CURSES
	libs += -lncurses
endif

ifeq ($(GRAPHICS),YES)
	cflags += $(shell $(SDL_CONFIG) --cflags)
	cppflags += -DBROGUE_TCOD -I$(TCOD_DIR)/include
	libs += -L$(TCOD_DIR) -ltcod $(shell $(SDL_CONFIG) --libs)
endif

ifeq ($(DEBUG),YES)
	cflags += -g
	cppflags += -DDEBUGGING=1
else
	cflags += -O2
endif

.PHONY: clean

%.o: %.c src/brogue/Rogue.h src/brogue/IncludeGlobals.h
	$(CC) $(cppflags) $(CPPFLAGS) $(cflags) $(CFLAGS) -c $< -o $@

bin/brogue: $(objects)
	$(CC) $(LDFLAGS) $(libs) $(LDLIBS) $(cflags) $(CFLAGS) $^ -o bin/brogue

clean:
	$(RM) $(objects) bin/brogue
