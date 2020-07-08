include config.mk

cflags := -Isrc/brogue -Isrc/platform -std=c99 \
	-Wall -Wpedantic -Werror=implicit -Wno-parentheses -Wno-unused-result -Wno-format
libs := -lm
cppflags := -DDATADIR=$(DATADIR)

sources := $(wildcard src/brogue/*.c) $(addprefix src/platform/,main.c platformdependent.c)

ifeq ($(TERMINAL),YES)
	sources += $(addprefix src/platform/,curses-platform.c term.c)
	cppflags += -DBROGUE_CURSES
	libs += -lncurses
endif

ifeq ($(GRAPHICS),YES)
	sources += $(addprefix src/platform/,sdl2-platform.c)
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
	cflags += -g
else
	cflags += -O2
endif

objects := $(sources:.c=.o)

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
	$(RM) src/brogue/*.o src/platform/*.o bin/brogue{,.exe}


common-files := README.txt CHANGELOG.txt LICENSE.txt seed-catalog.txt
common-bin := bin/assets bin/keymap.txt

%.txt: %.md
	cp $< $@

windows.zip: $(common-files) $(common-bin)
	zip -rvl $@ $^ bin/brogue.exe bin/*.dll

macos.zip: $(common-files)
	chmod +x "Brogue CE.app"/Contents/MacOS/brogue
	zip -rv -ll $@ $^ "Brogue CE.app"

linux.tar.gz: $(common-files) $(common-bin) brogue
	chmod +x bin/brogue
	tar -cavf $@ $^ bin/brogue -C linux make-link-for-desktop.sh


# $* is the matched %
icon_%.png: bin/assets/icon.png
	convert $< -resize $* $@

macos/Brogue.icns: icon_32.png icon_128.png icon_256.png icon_512.png
	png2icns $@ $^
	$(RM) $^

Brogue.app: bin/brogue
	mkdir -p $@/Contents/{MacOS,Resources}
	cp macos/Info.plist $@/Contents
	cp bin/brogue $@/Contents/MacOS
	cp -r macos/Brogue.icns bin/assets $@/Contents/Resources
