include config.mk

CFLAGS := -Isrc/brogue -Isrc/platform -Wall -Wno-parentheses
LDLIBS := -lm

sources := $(wildcard src/brogue/*.c) $(wildcard src/platform/*.c)
objects := $(sources:.c=.o)

ifeq ($(TERMINAL),yes)
	CPPFLAGS += -DBROGUE_CURSES
	LDLIBS += -lncurses
endif

.PHONY: clean

%.o: %.c src/brogue/Rogue.h src/brogue/IncludeGlobals.h

bin/brogue: $(objects)
	$(CC) $(LDFLAGS) $(LDLIBS) $(CFLAGS) $^ -o bin/brogue

clean:
	$(RM) $(objects) bin/brogue
