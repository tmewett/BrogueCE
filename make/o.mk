$(sources:.c=.o): %.o: %.c src/brogue/Rogue.h src/brogue/Globals.h src/brogue/GlobalsBase.h vars/cppflags vars/cflags make/o.mk
	$(CC) $(cppflags) $(cflags) -c $< -o $@
