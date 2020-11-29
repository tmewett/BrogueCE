include config.mk

cflags := -Isrc/brogue -Isrc/platform -std=c99 \
	-Wall -Wpedantic -Werror=implicit -Wno-parentheses -Wno-unused-result -Wno-format
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

binary := bin/brogue
objects := $(sources:.c=.o)

ifeq ($(MAC_APP),YES)
	cppflags += -DSDL_PATHS
endif

ifeq ($(WINDOWS_APP),YES)
	binary := bin/brogue.exe
	objects += windows/icon.o
endif

ifeq ($(DEBUG),YES)
	cflags += -g -Og
	cppflags += -DENABLE_PLAYBACK_SWITCH
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

bin/brogue.exe: $(objects)
	$(CC) $(cflags) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(libs) $(LDLIBS)
	mt -manifest windows/brogue.exe.manifest '-outputresource:bin/brogue.exe;1'

clean:
	$(RM) src/brogue/*.o src/platform/*.o $(binary) \
	  seed-catalog.txt README.txt CHANGELOG.txt \
	  macos.zip linux.tar.gz
	$(RM) -r Brogue\ CE.app

common-files := README.txt CHANGELOG.txt LICENSE.txt seed-catalog.txt
common-bin := bin/assets bin/keymap.txt

%.txt: %.md
	cp $< $@

seed-catalog.txt: $(binary)
	$(binary) --print-seed-catalog > $@

windows.zip: $(common-files) $(common-bin)
	zip -rvl $@ $^ $(binary) bin/*.dll bin/brogue-cmd.bat

# Filter-out problematic names so we can quote them manually
macos.zip: $(common-files) Brogue\ CE.app
	chmod +x "Brogue CE.app"/Contents/MacOS/brogue
	cd "Brogue CE.app"/Contents && dylibbundler -cd -b -x MacOS/brogue
	zip -rv -ll $@ $(filter-out Brogue CE.app,$^) "Brogue CE.app"

linux.tar.gz: $(common-files) $(common-bin) brogue
	chmod +x bin/brogue
	tar -cavf $@ $^ bin/brogue -C linux make-link-for-desktop.sh

# $* is the matched %
icon_%.png: bin/assets/icon.png
	convert $< -resize $* $@

macos/Brogue.icns: icon_32.png icon_128.png icon_256.png icon_512.png
	png2icns $@ $^
	$(RM) $^

Brogue\ CE.app: bin/brogue
	mkdir -p "$@"/Contents/MacOS
	mkdir -p "$@"/Contents/Resources
	cp macos/Info.plist "$@"/Contents
	cp bin/brogue "$@"/Contents/MacOS
	cp -r macos/Brogue.icns bin/assets "$@"/Contents/Resources
