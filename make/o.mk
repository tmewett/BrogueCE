$(sources:%.c=build/%.o): build/%.o: %.c src/brogue/Rogue.h src/brogue/Globals.h src/brogue/GlobalsBase.h vars/cppflags vars/cflags make/o.mk
	$(CC) $(cppflags) $(cflags) -c $< -o $@

build/src/variants/GlobalsBrogue.o build/src/variants/GlobalsRapidBrogue.o build/src/variants/GlobalsBulletBrogue.o: vars/extra_version
build/src/variants/GlobalsBrogue.o build/src/variants/GlobalsRapidBrogue.o build/src/variants/GlobalsBulletBrogue.o: cppflags += -DBROGUE_EXTRA_VERSION='"$(extra_version)"'
