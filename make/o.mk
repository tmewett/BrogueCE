$(sources:.c=.o): %.o: %.c src/brogue/Rogue.h src/brogue/IncludeGlobals.h src/brogue/IncludeGlobalsBase.h vars/cppflags vars/cflags make/o.mk
	$(CC) $(cppflags) $(cflags) -c $< -o $@
