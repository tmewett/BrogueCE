dirs:
	mkdir -p build/src/brogue
	mkdir -p build/src/platform
	mkdir -p build/src/variants

bin/brogue bin/brogue.exe: dirs $(objects) vars/cflags vars/LDFLAGS vars/libs vars/objects make/brogue.mk
	$(CC) $(cflags) $(LDFLAGS) -o $@ $(objects) $(libs)
