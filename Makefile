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

ifeq ($(MAC_APP),YES)
	cppflags += -DSDL_PATHS
endif

ifeq ($(DEBUG),YES)
	cflags += -g -Og
	cppflags += -DENABLE_PLAYBACK_SWITCH
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
	mt -manifest windows/brogue.exe.manifest '-outputresource:bin/brogue.exe;1'

clean:
	$(RM) src/brogue/*.o src/platform/*.o windows/icon.o bin/brogue{,.exe}


# Release archives

common_bin := bin/assets bin/keymap.txt

define make_release_base
	mkdir $@
	cp README.md $@/README.txt
	cp CHANGELOG.md $@/CHANGELOG.txt
	cp LICENSE.txt $@
endef

# Flatten bin/ in the Windows archive
BrogueCE-windows: bin/brogue.exe
	$(make_release_base)
	cp -r $(common_bin) bin/{brogue.exe,brogue-cmd.bat} $@

BrogueCE-macos: Brogue.app
	$(make_release_base)
	cp -r Brogue.app $@/"Brogue CE.app"

BrogueCE-linux: bin/brogue
	$(make_release_base)
	cp brogue $@
	cp -r --parents $(common_bin) bin/brogue $@
	cp linux/make-link-for-desktop.sh $@


# macOS app bundle

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
