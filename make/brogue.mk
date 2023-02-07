bin/brogue: $(objects) vars/cflags vars/LDFLAGS vars/libs vars/objects make/brogue.mk
	$(CC) $(cflags) $(LDFLAGS) -o $@ $(objects) $(libs)
