$(sources:.c=.o): %.o: %.c src/brogue/Rogue.h src/brogue/Globals.h src/brogue/GlobalsBase.h vars/cppflags vars/cflags make/o.mk
	$(CC) $(cppflags) $(cflags) -c $< -o $@

src/variants/GlobalsBrogue.o src/variants/GlobalsRapidBrogue.o: vars/extra_version
src/variants/GlobalsBrogue.o src/variants/GlobalsRapidBrogue.o: cppflags += -DBROGUE_EXTRA_VERSION='"$(extra_version)"'
