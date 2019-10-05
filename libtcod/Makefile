SDL_CONFIG := sdl-config

cflags := -O2 -Iinclude -Wall $(shell $(SDL_CONFIG) --cflags)
cppflags := -DLIBTCOD_EXPORTS -DNO_OPENGL
libs := $(shell $(SDL_CONFIG) --libs)
sources := $(wildcard src/*.c) src/png/lodepng.c

objects := $(sources:.c=.o)

.PHONY: clean

%.o: %.c
	$(CC) $(cppflags) $(CPPFLAGS) $(cflags) $(CFLAGS) -c $< -o $@

libtcod.so: $(objects)
	$(CC) $(cflags) $(CFLAGS) $^ -o $@ -shared -Wl,-soname,libtcod.so.1 $(LDFLAGS) $(libs) $(LDLIBS)

tcod.dll: $(objects)
	$(CC) $(cflags) $(CFLAGS) $^ -o $@ -shared $(LDFLAGS) $(libs) $(LDLIBS)

clean:
	$(RM) $(objects) libtcod.so tcod.dll
