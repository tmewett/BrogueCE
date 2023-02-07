$(objects): %.o: %.c src/brogue/Rogue.h src/brogue/IncludeGlobals.h vars/cppflags vars/cflags recipes/o.mk
	$(CC) $(cppflags) $(cflags) -c $< -o $@
