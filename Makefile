include config.mk

cflags := -Isrc/brogue -Isrc/platform -std=c99 \
	-Wall -Wpedantic -Werror=implicit -Wno-parentheses -Wno-unused-result -Wno-format
libs := -lm
cppflags := -DDATADIR=$(DATADIR)

sources := $(wildcard src/brogue/*.c) $(wildcard src/platform/*.c)
objects := $(sources:.c=.o)

ifeq ($(TERMINAL),YES)
	cppflags += -DBROGUE_CURSES
	libs += -lncurses
endif

ifeq ($(GRAPHICS),YES)
	cflags += $(shell $(SDL_CONFIG) --cflags)
	cppflags += -DBROGUE_SDL
	libs += $(shell $(SDL_CONFIG) --libs) -lSDL2_image
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
	$(CC) $(cflags) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(libs) $(LDLIBS)

windows/icon.o: windows/icon.rc
	windres $< $@

bin/brogue.exe: $(objects) windows/icon.o
	$(CC) $(cflags) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(libs) $(LDLIBS)

clean:
	$(RM) $(objects) bin/brogue{,.exe}


common-files := bin/assets bin/keymap.txt README.txt CHANGELOG.txt agpl.txt seed-catalog.txt

%.txt: %.md
	cp $< $@

windows.zip: $(common-files) bin/brogue.exe
	zip -rvl $@ $^

macos.tar.gz: $(common-files) brogue bin/brogue
	tar -cavf $@ $^

linux.tar.gz: $(common-files) brogue bin/brogue
	tar -cavf $@ $^ -C linux make-link-for-desktop.sh
