
SDL_FLAGS = `sdl-config --cflags` `sdl-config --libs`
LIBTCODDIR=src/libtcod-1.5.2
CFLAGS=-Isrc/brogue -Isrc/platform -Wall -Wno-parentheses ${DEFINES}
RELEASENAME=brogue-1.7.4
LASTTARGET := $(shell ./brogue --target)
CC ?= gcc

ifeq (${LASTTARGET},both)
all : both
else ifeq (${LASTTARGET},curses)
all : curses
else ifeq (${LASTTARGET},tcod)
all : tcod
else
all : both
endif

%.o : %.c Makefile src/brogue/Rogue.h src/brogue/IncludeGlobals.h
	$(CC) $(CFLAGS) -g -o $@ -c $< 

BROGUEFILES=src/brogue/Architect.o \
	src/brogue/Combat.o \
	src/brogue/Dijkstra.o \
	src/brogue/Globals.o \
	src/brogue/IO.o \
	src/brogue/Items.o \
	src/brogue/Light.o \
	src/brogue/Monsters.o \
	src/brogue/Buttons.o \
	src/brogue/Movement.o \
	src/brogue/Recordings.o \
	src/brogue/RogueMain.o \
	src/brogue/Random.o \
	src/brogue/MainMenu.o \
	src/brogue/Grid.o \
	src/brogue/Time.o \
	src/brogue/PowerTables.o \
	src/brogue/Sqrt.o \
	src/platform/main.o \
	src/platform/platformdependent.o \
	src/platform/curses-platform.o \
	src/platform/tcod-platform.o \
	src/platform/term.o

TCOD_DEF = -DBROGUE_TCOD -I$(LIBTCODDIR)/include
TCOD_DEP = ${LIBTCODDIR}
TCOD_LIB = -L. -L${LIBTCODDIR} ${SDL_FLAGS} -ltcod -Wl,-rpath,.

CURSES_DEF = -DBROGUE_CURSES
CURSES_LIB = -lncurses -lm


tcod : DEPENDENCIES += ${TCOD_DEP}
tcod : DEFINES += ${TCOD_DEF}
tcod : LIBRARIES += ${TCOD_LIB}

curses : DEFINES = ${CURSES_DEF}
curses : LIBRARIES = ${CURSES_LIB}

both : DEPENDENCIES += ${TCOD_DEP}
both : DEFINES += ${TCOD_DEF} ${CURSES_DEF}
both : LIBRARIES += ${TCOD_LIB} ${CURSES_LIB}

ifeq (${LASTTARGET},both)
both : bin/brogue
tcod : clean bin/brogue
curses : clean bin/brogue
else ifeq (${LASTTARGET},curses)
curses : bin/brogue
tcod : clean bin/brogue
both : clean bin/brogue
else ifeq (${LASTTARGET},tcod)
tcod : bin/brogue
curses : clean bin/brogue
both : clean bin/brogue
else
both : bin/brogue
curses : bin/brogue
tcod : bin/brogue
endif

.PHONY : clean both curses tcod tar

bin/brogue : ${DEPENDENCIES} ${BROGUEFILES}
	$(CC) -O2 -march=i586 -o bin/brogue ${BROGUEFILES} ${LIBRARIES} -Wl,-rpath,.

clean : 
	rm -f src/brogue/*.o src/platform/*.o bin/brogue

${LIBTCODDIR} :
	src/get-libtcod.sh

tar : both
	rm -f ${RELEASENAME}.tar.gz
	tar --transform 's,^,${RELEASENAME}/,' -czf ${RELEASENAME}.tar.gz \
	Makefile \
	brogue \
	$(wildcard *.sh) \
	$(wildcard *.rtf) \
	readme \
	$(wildcard *.txt) \
	bin/brogue \
	bin/keymap \
	bin/icon.bmp \
	bin/brogue-icon.png \
	$(wildcard bin/fonts/*.png) \
	$(wildcard bin/*.so) \
	$(wildcard src/*.sh) \
	$(wildcard src/brogue/*.c) \
	$(wildcard src/brogue/*.h) \
	$(wildcard src/platform/*.c) \
	$(wildcard src/platform/*.h)

