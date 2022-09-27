include config.mk

cflags := -Isrc/brogue -Isrc/platform -std=c99 \
	-Wall -Wpedantic -Werror=implicit -Wno-parentheses -Wno-unused-result \
	-Wformat -Werror=format-security -Wformat-overflow=0
libs := -lm 
cppflags := -DDATADIR=$(DATADIR)
ifeq ($(OS2_APP),YES)
	cflags += -D__ST_MT_ERRNO__ -O2 -march=pentium4 -mtune=pentium4 -Wall -Zmt -Wno-narrowing
	cppflags += -D__ST_MT_ERRNO__ -O2 -march=pentium4 -mtune=pentium4 -Wall -Zmt -Wno-narrowing
	libs += -lcx -lc -Zomf -Zbin-files -Zargs-wild -Zargs-resp -Zhigh-mem -Zstack 8000 -D__ST_MT_ERRNO__
endif

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
	ifeq ($(OS2_APP),YES)
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

os2/icon.res: os2/icon.rc
	wrc -qr $< -fo=$@

os2/brogue.lib: os2/brogue.def
	emximp -o $@ $<
	
bin/brogue_os2.exe: $(objects) os2/icon.res os2/brogue.lib
	$(CC) $(cflags) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(libs) $(LDLIBS)


windows/icon.o: windows/icon.rc
	windres $< $@

bin/brogue.exe: $(objects) windows/icon.o
	$(CC) $(cflags) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(libs) $(LDLIBS)
	mt -manifest windows/brogue.exe.manifest '-outputresource:bin/brogue.exe;1'


clean:
	$(RM) src/brogue/*.o src/platform/*.o windows/icon.o bin/brogue{,_os2}{,.exe} \
	os2/icon.res os2/brogue.lib


# Release archives

common_bin := bin/assets bin/keymap.txt

define make_release_base
	mkdir -p $@
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

ifeq ($(OS2_APP),YES)
BrogueCE-os2: bin/brogue_os2.exe
	$(make_release_base)
	cp -r $(common_bin) bin/brogue_os2.exe $@
endif


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

macos/sdl2.rb:
	curl -L 'https://raw.githubusercontent.com/Homebrew/homebrew-core/master/Formula/sdl2.rb' >$@
	patch $@ macos/sdl2-deployment-target.patch
